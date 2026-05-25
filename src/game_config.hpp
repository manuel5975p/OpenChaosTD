#pragma once

#include <string>

class JsonStore;

struct GameConfig {
    int gameWidth = 1200;
    int gameHeight = 1200;
    int fps = 120;
    float hudScale = 1.0f;
    std::string title = "OpenChaos TD";

    void Load(JsonStore& jsonio);
    void ApplyIcon();
};
