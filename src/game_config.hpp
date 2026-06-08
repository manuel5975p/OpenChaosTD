#pragma once

#include <string>

class FileStore;

struct GameConfig {
    int gameWidth = 1200;
    int gameHeight = 1200;
    int fps = 120;
    float hudScale = 1.0f;
    std::string title = "OpenChaos TD";
    float musicVolume = 1.0f;
    float sfxVolume = 1.0f;

    void Load(FileStore& fileStore);
    void Save(FileStore& fileStore);
    void ApplyIcon();
};
