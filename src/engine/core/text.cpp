#include "text.hpp"

#include "text_renderer.hpp"

#include <engine/util/file_store.hpp>

#include <toml++/toml.hpp>

#include <array>
#include <cstddef>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// Generated at build time from the TTFs in resources/fonts/ (cmake/embed_resource.cmake)
extern const unsigned char gVictorMonoTtf[];
extern const std::size_t gVictorMonoTtfSize;
extern const unsigned char gEBGaramondTtf[];
extern const std::size_t gEBGaramondTtfSize;

namespace {

std::unique_ptr<TextRenderer> sRenderer;
std::array<TextRenderer::FontId, Text::KindCount> sKindFont{};
TextRenderer::FontId sFallbackFont = 0; // embedded EB Garamond; always valid when sRenderer

// Embedded fonts selectable by canonical name (the fallback when no file override exists).
struct Embedded {
    std::string_view  name;
    const unsigned char* data;
    const std::size_t*   size;
};
const Embedded kEmbedded[] = {
    {"EBGaramond", gEBGaramondTtf, &gEBGaramondTtfSize},
    {"VictorMono", gVictorMonoTtf, &gVictorMonoTtfSize},
};

const Embedded* FindEmbedded(std::string_view name) {
    for (const Embedded& e : kEmbedded)
        if (e.name == name)
            return &e;
    return nullptr;
}

// Common font-search location for both the GPU and raylib loaders. Kept in sync with
// CMake's FONTS_DIR and the web --preload-file mapping (resources@resources).
constexpr std::string_view kFontDir = "resources/fonts/";

// Resolves a font name to a FontId: resources/fonts/<name>.{ttf,otf} file override
// first (via FileStore), then an embedded canonical name, else the fallback font.
// Loaded fonts are cached so kinds sharing a name share one FontId.
TextRenderer::FontId ResolveFont(FileStore& fileStore, const std::string& name,
                                 std::unordered_map<std::string, TextRenderer::FontId>& cache) {
    if (auto it = cache.find(name); it != cache.end())
        return it->second;

    for (std::string_view ext : {".ttf", ".otf"}) {
        const std::string path = std::string(kFontDir) + name + std::string(ext);
        if (!fileStore.Exists(path))
            continue;
        // FullPath keeps path resolution inside FileStore; LoadFont reads the file.
        auto id = sRenderer->LoadFont(fileStore.FullPath(path));
        if (id) {
            cache.emplace(name, *id);
            return *id;
        }
        TraceLog(LOG_WARNING, "FONTS: failed to load '%s' (%s) — falling back", path.c_str(),
                 id.error().c_str());
        break;
    }

    if (const Embedded* e = FindEmbedded(name)) {
        auto id = sRenderer->LoadFontFromMemory(e->data, *e->size);
        if (id) {
            cache.emplace(name, *id);
            return *id;
        }
        TraceLog(LOG_WARNING, "FONTS: embedded '%s' failed (%s) — falling back",
                 std::string(name).c_str(), id.error().c_str());
    } else {
        TraceLog(LOG_WARNING, "FONTS: unknown font '%s' (no file or embedded match) — falling back",
                 name.c_str());
    }

    cache.emplace(name, sFallbackFont);
    return sFallbackFont;
}

TextRenderer::FontId FontFor(Text::Kind kind) {
    return sKindFont[static_cast<int>(kind)];
}

// --- font name configuration -------------------------------------------------
// One font name per Text::Kind: a file stem under resources/fonts/ ("MyFont" ->
// resources/fonts/MyFont.ttf) or an embedded canonical name ("EBGaramond"/"VictorMono").
// Read from config/fonts.toml; missing file/keys keep the defaults. This is an
// implementation detail of the text subsystem (formerly the standalone FontConfig),
// so it lives here rather than as a public core header.
using FontNames = std::array<std::string, Text::KindCount>;

FontNames DefaultFontNames() {
    FontNames d;
    d.fill("EBGaramond");                                    // prose everywhere...
    d[static_cast<int>(Text::Kind::Number)] = "VictorMono";  // ...mono only for numbers
    return d;
}

FontNames LoadFontNames(FileStore& fileStore) {
    static constexpr const char* kKeys[Text::KindCount] = {
        "title", "heading", "body", "label", "number", "button", "tooltip",
    };
    FontNames names = DefaultFontNames();
    if (!fileStore.Exists("config/fonts.toml"))
        return names; // keep defaults

    const toml::table table = fileStore.LoadToml("config/fonts.toml");
    const toml::table* fonts = table["fonts"].as_table();
    if (fonts == nullptr) {
        TraceLog(LOG_WARNING, "FONTS: config/fonts.toml has no [fonts] table — using defaults");
        return names;
    }
    for (int k = 0; k < Text::KindCount; ++k)
        if (auto name = (*fonts)[kKeys[k]].value<std::string>(); name && !name->empty())
            names[k] = *name;
    return names;
}

// --- raylib bitmap fallback --------------------------------------------------
// The GPU (Slug) renderer needs desktop GL 3.3 and is unavailable on web. Rather
// than drop to raylib's built-in default font, we load the real project TTFs as
// raylib Fonts and draw them with DrawTextEx, so the typeface still looks right.

bool sRaylibFallback = false;
constexpr int kRayFontBaseSize = 64;     // rasterization size; DrawTextEx scales from here
std::array<Font, Text::KindCount> sKindRayFont{};
std::vector<Font> sOwnedRayFonts;        // distinct loaded fonts to UnloadFont() on shutdown

// Loads a raylib Font for a name: resources/fonts/<name>.{ttf,otf} from the (possibly
// preloaded) VFS first, then the embedded bytes, else raylib's default font. Cached so
// kinds sharing a name share one Font; newly loaded fonts are tracked for cleanup.
Font ResolveRayFont(FileStore& fileStore, const std::string& name,
                    std::unordered_map<std::string, Font>& cache) {
    if (auto it = cache.find(name); it != cache.end())
        return it->second;

    Font font{};
    bool loaded = false;
    for (std::string_view ext : {".ttf", ".otf"}) {
        const std::string path = std::string(kFontDir) + name + std::string(ext);
        if (!fileStore.Exists(path))
            continue;
        font = ::LoadFontEx(fileStore.FullPath(path).c_str(), kRayFontBaseSize, nullptr, 0);
        loaded = font.texture.id != 0 && font.glyphCount > 0;
        break;
    }
    if (!loaded) {
        if (const Embedded* e = FindEmbedded(name)) {
            font = ::LoadFontFromMemory(".ttf", e->data, static_cast<int>(*e->size),
                                        kRayFontBaseSize, nullptr, 0);
            loaded = font.texture.id != 0 && font.glyphCount > 0;
        }
    }
    if (!loaded) {
        TraceLog(LOG_WARNING, "FONTS(raylib): '%s' unavailable — using default font", name.c_str());
        font = ::GetFontDefault();
    } else {
        ::SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR); // smooth when up/down-scaled
        sOwnedRayFonts.push_back(font);
    }
    cache.emplace(name, font);
    return font;
}

// Builds the per-kind raylib Font table from the same font names the GPU path uses.
void InitRaylibFallback(FileStore& fileStore, const FontNames& names) {
    std::unordered_map<std::string, Font> cache;
    for (int k = 0; k < Text::KindCount; ++k)
        sKindRayFont[k] = ResolveRayFont(fileStore, names[k], cache);
    sRaylibFallback = true;
}

} // namespace

namespace Text {

void Init(FileStore& fileStore) {
    const FontNames fonts = LoadFontNames(fileStore);

    auto renderer = TextRenderer::Create();
    if (!renderer) {
        // GPU path unavailable (e.g. web): use real TTFs via raylib instead of DrawText.
        TraceLog(LOG_WARNING, "TEXT: %s — using raylib font fallback", renderer.error().c_str());
        InitRaylibFallback(fileStore, fonts);
        return;
    }
    sRenderer = std::move(*renderer);

    // Guaranteed fallback typeface: embedded EB Garamond. If even this fails the
    // GPU path is unusable, so disarm it and use the raylib font fallback instead.
    auto fallback = sRenderer->LoadFontFromMemory(gEBGaramondTtf, gEBGaramondTtfSize);
    if (!fallback) {
        TraceLog(LOG_WARNING, "TEXT: embedded font failed (%s) — using raylib font fallback",
                 fallback.error().c_str());
        sRenderer.reset();
        InitRaylibFallback(fileStore, fonts);
        return;
    }
    sFallbackFont = *fallback;

    std::unordered_map<std::string, TextRenderer::FontId> cache;
    cache.emplace("EBGaramond", sFallbackFont);

    for (int k = 0; k < KindCount; ++k)
        sKindFont[k] = ResolveFont(fileStore, fonts[k], cache);
}

void Shutdown() {
    sRenderer.reset();
    if (sRaylibFallback) {
        for (Font& f : sOwnedRayFonts) // distinct fonts only; GetFontDefault() was never tracked
            ::UnloadFont(f);
        sOwnedRayFonts.clear();
        sKindRayFont = {};
        sRaylibFallback = false;
    }
}

void Draw(const char* text, int x, int y, int fontSize, Color color, Kind kind) {
    if (sRenderer) {
        sRenderer->DrawText(FontFor(kind), text,
                            {static_cast<float>(x), static_cast<float>(y)},
                            static_cast<float>(fontSize), color);
        return;
    }
    if (sRaylibFallback) {
        ::DrawTextEx(sKindRayFont[static_cast<int>(kind)], text,
                     {static_cast<float>(x), static_cast<float>(y)},
                     static_cast<float>(fontSize), 0.0f, color);
        return;
    }
    ::DrawText(text, x, y, fontSize, color);
}

int Measure(const char* text, int fontSize, Kind kind) {
    if (sRenderer) {
        const Vector2 size = sRenderer->MeasureText(FontFor(kind), text, static_cast<float>(fontSize));
        return static_cast<int>(size.x + 0.5f);
    }
    if (sRaylibFallback) {
        const Vector2 size = ::MeasureTextEx(sKindRayFont[static_cast<int>(kind)], text,
                                             static_cast<float>(fontSize), 0.0f);
        return static_cast<int>(size.x + 0.5f);
    }
    return ::MeasureText(text, fontSize);
}

std::vector<std::string> Wrap(const std::string& text, float maxWidth, int fontSize, Kind kind) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string word, current;
    while (stream >> word) {
        std::string candidate = current.empty() ? word : current + " " + word;
        if (Measure(candidate.c_str(), fontSize, kind) <= static_cast<int>(maxWidth))
            current = candidate;
        else {
            if (!current.empty()) lines.push_back(current);
            current = word;
        }
    }
    if (!current.empty()) lines.push_back(current);
    return lines;
}

} // namespace Text
