#pragma once

#include <engine/core/text.hpp>

#include <array>
#include <string>

class FileStore;

// Maps each Text::Kind to a font name — a file stem under assets/fonts/ (e.g.
// "MyFont" -> assets/fonts/MyFont.ttf) or an embedded canonical name
// ("EBGaramond"/"VictorMono"). Loaded from config/fonts.toml; an absent file or
// missing keys keep the built-in defaults, so the map is always usable.
struct FontConfig {
    std::array<std::string, Text::KindCount> fontByKind = Defaults();

    // Fills fontByKind from the [fonts] table in config/fonts.toml via fileStore.
    // Best-effort: missing/malformed entries retain their default.
    void Load(FileStore& fileStore);

    // Built-in default mapping (the historical Prose/Mono split).
    static std::array<std::string, Text::KindCount> Defaults();

    // toml key for a Kind ("title", "heading", ...). kind in range.
    static const char* KeyFor(Text::Kind kind);
};
