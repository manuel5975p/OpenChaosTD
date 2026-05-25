#pragma once

#include <raylib.h>
#include <memory>
#include <states/game_state.hpp>
#include <engine/core/assets.hpp>
#include <engine/features/particle_system.hpp>
#include <engine/core/screen.hpp>
#include <engine/core/input.hpp>
#include <engine/util/json_store.hpp>
#include <game_config.hpp>
#include <engine/util/profiler.hpp>
#include <factory/emitter_presets.hpp>
#include <factory/tower_factory.hpp>
#include <factory/enemy_factory.hpp>
#include <world/game_data.hpp>

class Game {
public:
    Game();
    ~Game();

    void Run();

    // Search path of a directory
    std::filesystem::path SearchFolderParentPath(const std::string& folderName, size_t searchDepth);

    // State machine
    void ChangeState(std::unique_ptr<GameState> newState);
    void Quit() { m_running = false; }
    
    const GameConfig& GetGameConfig() const {return m_gameConfig;}
    GameData& GetGameData() {return m_gameData;}

    // Accessors for managers
    Assets& GetAssets() {return m_assets;}
    Screen& GetScreen() {return m_screen;}
    Input& GetInput() {return m_input;}
    JsonStore& GetJsonStore(){return m_jsonStore;}
    ParticleSystem& GetParticles() {return m_particles;}
    TowerFactory& GetTowerFactory() {return m_towerFactory;}
    EnemyFactory& GetEnemyFactory() {return m_enemyFactory;}

private:
    bool m_running = true;

    void LoadAssets();

    // Data and config
    GameConfig m_gameConfig;
    GameData m_gameData;

    // Core managers
    Assets m_assets;
    ParticleSystem m_particles;
    Screen m_screen;
    Input m_input;
    JsonStore m_jsonStore;
    Profiler m_profiler;
    EmitterPresets m_emitterPresets;
    TowerFactory m_towerFactory;
    EnemyFactory m_enemyFactory;

    // Gamestate management
    std::unique_ptr<GameState> m_currentState;
    std::unique_ptr<GameState> m_pendingState;
    void ApplyPendingState(); // Swaps in m_pendingState at safe point
};