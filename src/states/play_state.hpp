#pragma once

#include <states/game_state.hpp>
#include <raylib.h>
#include <systems/render_system.hpp>
#include <systems/world_system.hpp>
#include <systems/enemy_system.hpp>
#include <systems/tower_system.hpp>
#include <hud/tower_hud.hpp>
#include <hud/score_hud.hpp>
#include <hud/tower_info_hud.hpp>
#include <hud/event_log.hpp>
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

    struct SelectionContext {
        DenseSlotMap<Tower>::Key towerKey = DenseSlotMap<Tower>::INVALID_KEY;
        std::string hoveredTowerName; // name of the tower-bar button being previewed
    };

    bool m_debug = false;
    bool m_gameOver = false;

    Tower m_hoveredTowerCache; // rebuilt only when hoveredTowerName changes

    ScoreHUD m_scoreHUD;
    TowerHUD m_towerHUD;
    TowerInfoHUD m_towerInfoHUD;
    EventLog m_eventLog;

    SelectionContext m_selection;

    RenderSystem m_renderSystem;
    WorldSystem m_worldSystem;
    TowerSystem m_towerSystem;
    EnemySystem m_enemySystem;
    WaveManager m_waveManager;
};