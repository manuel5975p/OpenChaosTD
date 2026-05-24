#include <world/game_data.hpp>
#include <core/jsonio.hpp>

void GameData::Load(JsonIO& jsonio) {
    if (!jsonio.Exists("config/gameplay.json"))
        return;

    auto j = jsonio.Load("config/gameplay.json");
    if (j.contains("startingLives")) startingLives = j["startingLives"].get<int>();
    if (j.contains("startingGold")) startingGold = j["startingGold"].get<int>();
    if (j.contains("sellRefundRate")) sellRefundRate = j["sellRefundRate"].get<float>();
    if (j.contains("totalWaves")) totalWaves = j["totalWaves"].get<int>();
    if (j.contains("autoSpawnDelay")) autoSpawnDelay = j["autoSpawnDelay"].get<float>();

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
    m_payloads.clear();
    m_vfx.clear();
}
