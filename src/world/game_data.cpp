#include <world/game_data.hpp>
#include <core/jsonio.hpp>

void GameData::Load(JsonIO& jsonio) {
    if (!jsonio.Exists("data/gameplay.json"))
        return;

    auto j = jsonio.Load("data/gameplay.json");
    if (j.contains("startingLives")) startingLives = j["startingLives"].get<int>();
    if (j.contains("startingGold")) startingGold = j["startingGold"].get<int>();

    lives = startingLives;
    gold = startingGold;
}

void GameData::Reset() {
    lives = startingLives;
    gold = startingGold;
    score = 0;
    victory = false;
    waveNumber = 0;
    waveActive = false;
    waveTimer  = 0.0f;
    map = Map();
    towers = DenseSlotMap<Tower>();
    enemies = DenseSlotMap<Enemy>();
    attacks.clear();
}
