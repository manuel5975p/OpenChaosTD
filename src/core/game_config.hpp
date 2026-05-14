#pragma once

#include <string>

class JsonIO;

struct GameConfig {
    int gameWidth = 1200;
    int gameHeight = 1200;
    int fps = 120;
    std::string title = "OpenChaos TD";

    void Load(JsonIO& jsonio);
    void ApplyIcon();
};
