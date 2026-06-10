#include "text.hpp"

#include "font_config.hpp"
#include "text_renderer.hpp"

#include <engine/util/file_store.hpp>

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

// Generated at build time from the TTFs in assets/fonts/ (cmake/embed_resource.cmake)
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

// Resolves a font name to a FontId: assets/fonts/<name>.{ttf,otf} file override
// first (via FileStore), then an embedded canonical name, else the fallback font.
// Loaded fonts are cached so kinds sharing a name share one FontId.
TextRenderer::FontId ResolveFont(FileStore& fileStore, const std::string& name,
                                 std::unordered_map<std::string, TextRenderer::FontId>& cache) {
    if (auto it = cache.find(name); it != cache.end())
        return it->second;

    for (std::string_view ext : {".ttf", ".otf"}) {
        const std::string path = "assets/fonts/" + name + std::string(ext);
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

} // namespace

namespace Text {

void Init(FileStore& fileStore) {
    auto renderer = TextRenderer::Create();
    if (!renderer) {
        TraceLog(LOG_WARNING, "TEXT: %s — falling back to raylib DrawText", renderer.error().c_str());
        return;
    }
    sRenderer = std::move(*renderer);

    // Guaranteed fallback typeface: embedded EB Garamond. If even this fails the
    // GPU path is unusable, so disarm it and let Draw/Measure use raylib.
    auto fallback = sRenderer->LoadFontFromMemory(gEBGaramondTtf, gEBGaramondTtfSize);
    if (!fallback) {
        TraceLog(LOG_WARNING, "TEXT: embedded font failed (%s) — falling back to raylib DrawText",
                 fallback.error().c_str());
        sRenderer.reset();
        return;
    }
    sFallbackFont = *fallback;

    std::unordered_map<std::string, TextRenderer::FontId> cache;
    cache.emplace("EBGaramond", sFallbackFont);

    FontConfig config;
    config.Load(fileStore);
    for (int k = 0; k < KindCount; ++k)
        sKindFont[k] = ResolveFont(fileStore, config.fontByKind[k], cache);
}

void Shutdown() {
    sRenderer.reset();
}

void Draw(const char* text, int x, int y, int fontSize, Color color, Kind kind) {
    if (!sRenderer) {
        ::DrawText(text, x, y, fontSize, color);
        return;
    }
    sRenderer->DrawText(FontFor(kind), text,
                        {static_cast<float>(x), static_cast<float>(y)},
                        static_cast<float>(fontSize), color);
}

int Measure(const char* text, int fontSize, Kind kind) {
    if (!sRenderer)
        return ::MeasureText(text, fontSize);
    const Vector2 size = sRenderer->MeasureText(FontFor(kind), text, static_cast<float>(fontSize));
    return static_cast<int>(size.x + 0.5f);
}

} // namespace Text
