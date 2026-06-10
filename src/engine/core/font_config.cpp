#include <engine/core/font_config.hpp>

#include <engine/util/file_store.hpp>

#include <raylib.h>

namespace {

// Indexed by Text::Kind; mirrors the enum order.
constexpr const char* kKeys[Text::KindCount] = {
    "title", "heading", "body", "label", "number", "button", "tooltip",
};

} // namespace

std::array<std::string, Text::KindCount> FontConfig::Defaults() {
    // Historical Prose/Mono split: prose everywhere, mono only for numbers.
    std::array<std::string, Text::KindCount> d;
    d.fill("EBGaramond");
    d[static_cast<int>(Text::Kind::Number)] = "VictorMono";
    return d;
}

const char* FontConfig::KeyFor(Text::Kind kind) {
    return kKeys[static_cast<int>(kind)];
}

void FontConfig::Load(FileStore& fileStore) {
    if (!fileStore.Exists("config/fonts.toml"))
        return; // keep defaults

    const toml::table table = fileStore.LoadToml("config/fonts.toml");
    const toml::table* fonts = table["fonts"].as_table();
    if (fonts == nullptr) {
        TraceLog(LOG_WARNING, "FONTS: config/fonts.toml has no [fonts] table — using defaults");
        return;
    }

    for (int k = 0; k < Text::KindCount; ++k) {
        if (auto name = (*fonts)[kKeys[k]].value<std::string>(); name && !name->empty())
            fontByKind[k] = *name;
    }
}
