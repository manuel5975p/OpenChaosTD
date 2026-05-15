#include <states/play_state.hpp>

#include <states/end_state.hpp>
#include <game.hpp>
#include <raylib.h>
#include <raymath.h>


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
    if (game.GetInput().IsPressed("Debug")) m_debug = !m_debug;
    m_renderSystem.ControlCamera(dt, game.GetInput());

    HandleHUDInput(game);

    Vector2 mouseWorld = game.GetInput().GetWorldMousePosition(m_renderSystem.GetCamera());
    HandleTowerPlacement(game, mouseWorld);
    HandleTowerRemoval(game, mouseWorld);
}

void PlayingState::HandleHUDInput(Game& game) {
    // HUDs consume mouse input first so clicks don't bleed through to the world
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

    // Handle signals from HUDs — sell, wave start, auto-spawn toggle
    if (m_towerInfoHUD.WasSellRequested()) {
        if (Tower* tower = game.GetGameData().towers.Get(m_selectedTowerKey)) {
            int x, y;
            if (game.GetGameData().map.WorldToTile(tower->m_position, x, y)) {
                game.GetGameData().gold += static_cast<int>(tower->m_cost * game.GetGameData().sellRefundRate);
                m_worldSystem.RemoveTower(x, y, game.GetGameData());
            }
        }
        m_selectedTowerKey = DenseSlotMap<Tower>::INVALID_KEY;
    }

    if (m_scoreHUD.WasWaveRequested())
        m_waveManager.StartWave(game.GetGameData(), m_worldSystem, game.GetEnemyFactory());

    if (m_scoreHUD.WasAutoToggled())
        m_waveManager.ToggleAutoSpawn();
}

void PlayingState::HandleTowerPlacement(Game& game, Vector2 mouseWorld) {
    if (!game.GetInput().IsPressed("PlaceTower")) return;
    if (game.GetInput().IsMouseInputConsumed()) return;

    int x, y;
    if (!game.GetGameData().map.WorldToTile(mouseWorld, x, y)) {
        // Clicked outside the map — deselect
        m_selectedTowerKey = DenseSlotMap<Tower>::INVALID_KEY;
        return;
    }

    Tile& tile = game.GetGameData().map.Get(x, y);

    // Tile has a tower — select it regardless of wave state
    if (tile.m_towerKey != DenseSlotMap<Tower>::INVALID_KEY) {
        m_selectedTowerKey = tile.m_towerKey;
        return;
    }

    // Placement is blocked during waves or when no tower type is selected
    if (game.GetGameData().waveActive) return;
    if (m_towerHUD.GetSelectedTower().empty()) return;

    m_selectedTowerKey = DenseSlotMap<Tower>::INVALID_KEY;
    int cost = game.GetTowerFactory().GetCost(m_towerHUD.GetSelectedTower());
    if (game.GetGameData().gold >= cost) {
        Tower tower = game.GetTowerFactory().Create(m_towerHUD.GetSelectedTower());
        if (m_worldSystem.PlaceTower(x, y, tower, game.GetGameData())) {
            game.GetGameData().gold -= cost;
            m_towerHUD.ClearSelection();
        }
    }
}

void PlayingState::HandleTowerRemoval(Game& game, Vector2 mouseWorld) {
    if (!game.GetInput().IsPressed("RemoveTower")) return;
    if (game.GetInput().IsMouseInputConsumed()) return;
    if (game.GetGameData().waveActive) return;

    int x, y;
    if (!game.GetGameData().map.WorldToTile(mouseWorld, x, y)) return;

    Tile& tile = game.GetGameData().map.Get(x, y);
    if (tile.m_towerKey == m_selectedTowerKey)
        m_selectedTowerKey = DenseSlotMap<Tower>::INVALID_KEY;
    m_worldSystem.RemoveTower(x, y, game.GetGameData());
}

void PlayingState::Update(Game& game, float dt) {
    if (game.GetGameData().victory)
        game.ChangeState(std::make_unique<EndState>(true));

    if (m_gameOver)
        game.ChangeState(std::make_unique<EndState>(false));

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

        // Range indicator for the selected tower
        if (m_selectedTowerKey != DenseSlotMap<Tower>::INVALID_KEY) {
            if (const Tower* t = game.GetGameData().towers.Get(m_selectedTowerKey))
                m_renderSystem.DrawTowerRange(t->m_position, t->m_radius, {255, 200, 50, 220});
        }

        // Ghost tower preview: only when a tower type is selected in the HUD and no wave is active
        if (!game.GetGameData().waveActive && m_selectedTowerKey == DenseSlotMap<Tower>::INVALID_KEY
            && !m_towerHUD.GetSelectedTower().empty()) {
            Vector2 mouseWorld = game.GetInput().GetWorldMousePosition(m_renderSystem.GetCamera());
            int x, y;
            if (game.GetGameData().map.WorldToTile(mouseWorld, x, y)) {
                float half = game.GetGameData().map.GetTileSize() / 2.0f;
                Vector2 tileCenter = Vector2Add(game.GetGameData().map.TileToWorld(x, y), {half, half});
                const std::string& name = m_towerHUD.GetSelectedTower();
                Texture2D& tex = game.GetAssets().GetTexture(game.GetTowerFactory().GetTexture(name));
                m_renderSystem.DrawGhostTower(tileCenter, game.GetTowerFactory().GetRadius(name), tex);
            }
        }

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

