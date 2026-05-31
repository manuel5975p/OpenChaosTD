#include <game_config.hpp>
#include <engine/util/json_store.hpp>
#include <raylib.h>

void GameConfig::Load(JsonStore& jsonio) {
    if (!jsonio.Exists("config/settings.json"))
        return;

    auto j = jsonio.Load("config/settings.json");
    if (j.contains("gameWidth")) gameWidth = j["gameWidth"].get<int>();
    if (j.contains("gameHeight")) gameHeight = j["gameHeight"].get<int>();
    if (j.contains("fps")) fps = j["fps"].get<int>();
    if (j.contains("hudScale")) hudScale = j["hudScale"].get<float>();
    if (j.contains("title")) title = j["title"].get<std::string>();
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
