#include <world/game_data.hpp>
#include <engine/util/file_store.hpp>

void GameData::Load(FileStore& fileStore) {
    if (!fileStore.Exists("data/gameplay.toml"))
        return;

    auto data = fileStore.LoadToml("data/gameplay.toml");
    m_startingLives  = data["startingLives"].value_or(m_startingLives);
    m_startingGold   = data["startingGold"].value_or(m_startingGold);
    m_sellRefundRate = data["sellRefundRate"].value_or(m_sellRefundRate);
    m_autoSpawnDelay = data["autoSpawnDelay"].value_or(m_autoSpawnDelay);

    m_lives = m_startingLives;
    m_gold = m_startingGold;
}

void GameData::Reset() {
    m_lives = m_startingLives;
    m_gold = m_startingGold;
    m_victory = false;
    m_waveNumber = 0;
    m_waveActive = false;
    m_map = Map();
    m_towers = DenseSlotMap<Tower>();
    m_enemies = DenseSlotMap<Enemy>();
    m_attacks.clear();
}
