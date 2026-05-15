#include <states/play_state.hpp>

#include <states/end_state.hpp>
#include <game.hpp>
#include <raylib.h>


void PlayingState::OnEnter(Game& game) {
    game.GetGameData().Reset();

    m_worldSystem.GenerateMap(game.GetGameData().map, 15, 19);
    game.GetGameData().map.BuildPathMesh();

    m_renderSystem.CenterCamera(game.GetGameData().map, game.GetRenderer());

    m_scoreHUD.Build(game);
    m_towerHUD.Build(game);
}

void PlayingState::OnExit(Game& /*game*/) {}

void PlayingState::ProcessInput(Game& game, float dt) {
    // Toggle debug mode
    if(game.GetInput().IsPressed("Debug")) m_debug = !m_debug;

    // Control camera
    m_renderSystem.ControlCamera(dt, game.GetInput());

    // HUDs consume input first so clicks don't bleed through to the world
    m_scoreHUD.ProcessInput(game);
    m_towerHUD.ProcessInput(game);

    // Position and process the info HUD only while a tower is selected
    if (m_selectedTowerKey != DenseSlotMap<Tower>::INVALID_KEY) {
        if (Tower* tower = game.GetGameData().towers.Get(m_selectedTowerKey)) {
            Vector2 screenPos = GetWorldToScreen2D(tower->m_position, m_renderSystem.GetCamera());
            m_towerInfoHUD.SetAnchor(screenPos, game.GetRenderer().GetGameWidth(), game.GetRenderer().GetGameHeight(), *tower);
            m_towerInfoHUD.ProcessInput(game);
        }
    }

    // Sell: refund 50%, remove tower, deselect
    if (m_towerInfoHUD.WasSellRequested()) {
        if (Tower* tower = game.GetGameData().towers.Get(m_selectedTowerKey)) {
            int x, y;
            if (game.GetGameData().map.WorldToTile(tower->m_position, x, y)) {
                game.GetGameData().gold += tower->m_cost / 2;
                m_worldSystem.RemoveTower(x, y, game.GetGameData());
            }
        }
        m_selectedTowerKey = DenseSlotMap<Tower>::INVALID_KEY;
    }

    if (m_scoreHUD.WasWaveRequested())
        m_waveManager.StartWave(game.GetGameData(), m_worldSystem, game.GetEnemyFactory());

    if (m_scoreHUD.WasAutoToggled())
        m_waveManager.ToggleAutoSpawn();

    Vector2 mouseWorld = game.GetInput().GetWorldMousePosition(m_renderSystem.GetCamera());

    bool waveActive = game.GetGameData().waveActive;

    // Left-click: select an existing tower, or place a new one on empty ground (blocked during waves)
    if (game.GetInput().IsPressed("PlaceTower") && !game.GetInput().IsMouseInputConsumed()) {
        int x, y;
        if (game.GetGameData().map.WorldToTile(mouseWorld, x, y)) {
            Tile& tile = game.GetGameData().map.Get(x, y);
            if (tile.m_towerKey != DenseSlotMap<Tower>::INVALID_KEY) {
                // Tile has a tower — select it regardless of wave state
                m_selectedTowerKey = tile.m_towerKey;
            } else if (!waveActive) {
                // Empty tile and no active wave — deselect and attempt placement
                m_selectedTowerKey = DenseSlotMap<Tower>::INVALID_KEY;
                int cost = game.GetTowerFactory().GetCost(m_towerHUD.GetSelectedTower());
                if (game.GetGameData().gold >= cost) {
                    Tower tower = game.GetTowerFactory().Create(m_towerHUD.GetSelectedTower());
                    if (m_worldSystem.PlaceTower(x, y, tower, game.GetGameData()))
                        game.GetGameData().gold -= cost;
                }
            }
        } else {
            // Clicked outside the map — deselect
            m_selectedTowerKey = DenseSlotMap<Tower>::INVALID_KEY;
        }
    }

    // Right-click: remove tower (blocked during waves); deselect if it was selected
    if (!waveActive && game.GetInput().IsPressed("RemoveTower") && !game.GetInput().IsMouseInputConsumed()) {
        int x, y;
        if (game.GetGameData().map.WorldToTile(mouseWorld, x, y)) {
            Tile& tile = game.GetGameData().map.Get(x, y);
            if (tile.m_towerKey == m_selectedTowerKey)
                m_selectedTowerKey = DenseSlotMap<Tower>::INVALID_KEY;
            m_worldSystem.RemoveTower(x, y, game.GetGameData());
        }
    }
}

void PlayingState::Update(Game& game, float dt) {
    if (game.GetGameData().victory)
        game.ChangeState(std::make_unique<EndState>(true));

    if (m_gameOver)
        game.ChangeState(std::make_unique<EndState>(false));

    // Deselect if the selected tower was removed by something other than player input
    if (m_selectedTowerKey != DenseSlotMap<Tower>::INVALID_KEY) {
        if (!game.GetGameData().towers.Get(m_selectedTowerKey))
            m_selectedTowerKey = DenseSlotMap<Tower>::INVALID_KEY;
    }

    m_waveManager.Update(dt, game.GetGameData(), m_worldSystem, game.GetEnemyFactory());

    m_enemySystem.TickEnemies(dt, game.GetGameData());
    m_enemySystem.FollowPath(dt, game.GetGameData());

    m_towerSystem.update(dt, game.GetGameData());

    m_worldSystem.CheckEnemyReachedCore(game.GetGameData());
    m_towerSystem.TickAttacks(dt, game.GetGameData());
    m_worldSystem.CheckEnemyDead(game.GetGameData());
    m_worldSystem.CheckGameOver(m_gameOver, game.GetGameData());
}

void PlayingState::Draw(Game& game) {
    ClearBackground(DARKGRAY);

    BeginMode2D(m_renderSystem.GetCamera());
        m_renderSystem.DrawMap(game.GetGameData().map, game.GetAssets());
        m_renderSystem.DrawPaths(game.GetGameData().map);
        if(m_debug) {
            m_renderSystem.DebugDrawMap(game.GetGameData().map);
            m_renderSystem.DebugDrawEnemies(game.GetGameData().enemies);
        }
        m_renderSystem.DrawTowers(game.GetGameData().towers, game.GetAssets());

        m_renderSystem.DrawEnemies(game.GetGameData().enemies, game.GetAssets());
        m_renderSystem.DrawAttacks(game.GetGameData().attacks);

    EndMode2D();

    m_towerHUD.Draw(game);
    m_scoreHUD.Draw(game, m_waveManager.IsAutoSpawn());

    // Draw tower info panel if a tower is selected
    if (m_selectedTowerKey != DenseSlotMap<Tower>::INVALID_KEY) {
        if (const Tower* tower = game.GetGameData().towers.Get(m_selectedTowerKey))
            m_towerInfoHUD.Draw(game, *tower);
    }
}

