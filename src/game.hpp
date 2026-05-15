#pragma once

#include <raylib.h>
#include <memory>
#include <states/game_state.hpp>
#include <core/asset_manager.hpp>
#include <core/renderer.hpp>
#include <core/input_manager.hpp>
#include <core/jsonio.hpp>
#include <core/game_config.hpp>
#include <core/performance_monitor.hpp>
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
    AssetManager& GetAssets() {return m_assets;}
    Renderer& GetRenderer() {return m_renderer;}
    InputManager& GetInput() {return m_input;}
    JsonIO& GetJsonIO(){return m_jsonio;}
    PerformanceMonitor& GetMonitor() {return m_monitor;}
    TowerFactory& GetTowerFactory() {return m_towerFactory;}
    EnemyFactory& GetEnemyFactory() {return m_enemyFactory;}

private:
    bool m_running = true;

    void LoadAssets();

    // Data and config
    GameConfig m_gameConfig;
    GameData m_gameData;

    // Core managers
    AssetManager m_assets;
    Renderer m_renderer;
    InputManager m_input;
    JsonIO m_jsonio;
    PerformanceMonitor m_monitor;
    TowerFactory m_towerFactory;
    EnemyFactory m_enemyFactory;

    // Gamestate management
    std::unique_ptr<GameState> m_currentState;
    std::unique_ptr<GameState> m_pendingState;
    void ApplyPendingState(); // Swaps in m_pendingState at safe point
};