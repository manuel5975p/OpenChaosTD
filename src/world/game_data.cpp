#include <world/game_data.hpp>
#include <engine/util/json_store.hpp>

void GameData::Load(JsonStore& jsonio) {
    if (!jsonio.Exists("data/gameplay.json"))
        return;

    auto j = jsonio.Load("data/gameplay.json");
    if (j.contains("startingLives")) m_startingLives = j["startingLives"].get<int>();
    if (j.contains("startingGold")) m_startingGold = j["startingGold"].get<int>();
    if (j.contains("sellRefundRate")) m_sellRefundRate = j["sellRefundRate"].get<float>();
    if (j.contains("totalWaves")) m_totalWaves = j["totalWaves"].get<int>();
    if (j.contains("autoSpawnDelay")) m_autoSpawnDelay = j["autoSpawnDelay"].get<float>();

    m_lives = m_startingLives;
    m_gold = m_startingGold;
}

void GameData::Reset() {
    m_lives = m_startingLives;
    m_gold = m_startingGold;
    m_victory = false;
    m_waveNumber = 0;
    m_waveActive = false;
    m_waveTimer = 0.0f;
    m_map = Map();
    m_towers = DenseSlotMap<Tower>();
    m_enemies = DenseSlotMap<Enemy>();
    m_payloads.clear();
    m_vfx.clear();
}
