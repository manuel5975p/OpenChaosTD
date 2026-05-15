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
#include <systems/wave_manager.hpp>

class PlayingState : public GameState {
public:
    void OnEnter(Game& game) override;
    void OnExit(Game& game) override;

    void ProcessInput(Game& game, float dt) override;
    void Update(Game& game, float dt) override;
    void Draw(Game& game) override;

private:
    // ProcessInput helpers — each owns one concern
    void HandleHUDInput(Game& game);
    void HandleTowerPlacement(Game& game, Vector2 mouseWorld);
    void HandleTowerRemoval(Game& game, Vector2 mouseWorld);
    bool m_debug = false;
    bool m_gameOver = false;

    ScoreHUD     m_scoreHUD;
    TowerHUD     m_towerHUD;
    TowerInfoHUD m_towerInfoHUD;

    // Key of the tower the player has clicked to inspect; INVALID_KEY when nothing is selected
    DenseSlotMap<Tower>::Key m_selectedTowerKey = DenseSlotMap<Tower>::INVALID_KEY;
    RenderSystem m_renderSystem;
    WorldSystem  m_worldSystem;
    TowerSystem  m_towerSystem;
    EnemySystem  m_enemySystem;
    WaveManager  m_waveManager;
};