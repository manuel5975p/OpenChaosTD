#include <game_config.hpp>
#include <engine/util/file_store.hpp>
#include <raylib.h>

void GameConfig::Load(FileStore& fileStore) {
    if (!fileStore.Exists("config/settings.json"))
        return;

    auto j = fileStore.LoadJson("config/settings.json");

    // Window bootstrap group (not editable from the settings menu)
    if (j.contains("window")) {
        const auto& w = j["window"];
        if (w.contains("width")) gameWidth = w["width"].get<int>();
        if (w.contains("height")) gameHeight = w["height"].get<int>();
        if (w.contains("title")) title = w["title"].get<std::string>();
    }
    // Display group
    if (j.contains("display")) {
        const auto& d = j["display"];
        if (d.contains("fps")) fps = d["fps"].get<int>();
        if (d.contains("hudScale")) hudScale = d["hudScale"].get<float>();
    }
    // Audio group
    if (j.contains("audio")) {
        const auto& a = j["audio"];
        if (a.contains("musicVolume")) musicVolume = a["musicVolume"].get<float>();
        if (a.contains("sfxVolume")) sfxVolume = a["sfxVolume"].get<float>();
    }
}

void GameConfig::Save(FileStore& fileStore) {
    // Mirror the grouped on-disk shape. The window group is written straight from
    // the live struct so it round-trips untouched even though the menu never edits it.
    nlohmann::json j;
    j["window"]["width"] = gameWidth;
    j["window"]["height"] = gameHeight;
    j["window"]["title"] = title;
    j["display"]["fps"] = fps;
    j["display"]["hudScale"] = hudScale;
    j["audio"]["musicVolume"] = musicVolume;
    j["audio"]["sfxVolume"] = sfxVolume;
    fileStore.SaveJson("config/settings.json", j);
}

void GameConfig::ApplyIcon() {
    // 64x64 icon: dark background, stone tower, gold orb
    Image icon = GenImageColor(64, 64, {15, 15, 25, 255});

    // Tower base (wider footing)
    ImageDrawRectangle(&icon, 17, 50, 30, 8, {80, 80, 100, 255});
    // Tower body
    ImageDrawRectangle(&icon, 22, 18, 20, 34, {80, 80, 100, 255});
    // Battlements (3 merlons)
    ImageDrawRectangle(&icon, 22, 10, 5, 10, {80, 80, 100, 255});
    ImageDrawRectangle(&icon, 30, 10, 4, 10, {80, 80, 100, 255});
    ImageDrawRectangle(&icon, 37, 10, 5, 10, {80, 80, 100, 255});
    // Arrow slit window
    ImageDrawRectangle(&icon, 29, 28, 6, 12, {25, 25, 40, 255});
    // Gold orb behind the slit
    ImageDrawCircle(&icon, 32, 34, 4, {255, 195, 50, 255});
    // Re-draw slit over orb so it reads as a window glow
    ImageDrawRectangle(&icon, 29, 28, 6, 12, {255, 220, 100, 180});

    SetWindowIcon(icon);
    UnloadImage(icon);
}
