#pragma once

#include <states/game_state.hpp>
#include <raylib.h>
#include <systems/render_system.hpp>
#include <systems/world_system.hpp>
#include <systems/enemy_system.hpp>
#include <systems/tower_system.hpp>
#include <world/map_generator.hpp>
#include <hud/tower_build_hud.hpp>
#include <hud/status_hud.hpp>
#include <hud/tower_info_hud.hpp>
#include <hud/wave_hud.hpp>
#include <hud/event_hud.hpp>
#include <systems/wave_manager.hpp>
class PlayingState : public GameState {
public:
    void OnEnter(Game& game) override;
    void OnExit(Game& game) override;

    void ProcessInput(Game& game, float dt) override;
    void Update(Game& game, float dt) override;
    void Draw(Game& game) override;

private:
    // Input helpers
    void HandleHudSignals(Game& game);
    void HandleTowerPlacement(Game& game, Vector2 mouseWorld);
    void SyncHUDState(Game& game);
    void UpgradeSelectedTower(Game& game);

    // Game speed: run the simulation kSpeedSteps[m_speedIndex] times per frame
    void StepSimulation(Game& game, float dt);
    void CycleSpeed() { m_speedIndex = (m_speedIndex + 1) % static_cast<int>(sizeof(kSpeedSteps) / sizeof(kSpeedSteps[0])); }

    static constexpr int kSpeedSteps[] = {1, 2, 3};
    int m_speedIndex = 0;

    struct SelectionContext {
        DenseSlotMap<Tower>::Key towerKey = DenseSlotMap<Tower>::INVALID_KEY;
        std::string hoveredTowerName; // name of the tower-bar button being previewed
    };

    bool m_debug = false;
    bool m_gameOver = false;

    Tower m_hoveredTowerCache; // rebuilt only when hoveredTowerName changes

    StatusHUD m_scoreHUD;
    TowerBuildHUD m_towerHUD;
    TowerInfoHUD m_towerInfoHUD;
    WaveHUD m_waveHUD;
    EventHUD m_eventLog;

    SelectionContext m_selection;

    RenderSystem m_renderSystem;
    WorldSystem m_worldSystem;
    TowerSystem m_towerSystem;
    EnemySystem m_enemySystem;
    WaveManager m_waveManager;
    MapGenerator m_mapGenerator;
};