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

    m_towerHUD.Build(game);
    m_scoreHUD.Build(game);
    m_towerInfoHUD.Build(game);
    m_eventLog.Build(game);

    m_waveManager.Load(game.GetJsonIO());
}

void PlayingState::OnExit(Game& /*game*/) {}

void PlayingState::ProcessInput(Game& game, float dt) {
    if (game.GetInput().IsPressed("Debug")) m_debug = !m_debug;
    m_renderSystem.ControlCamera(dt, game.GetInput());

    // HUDs consume mouse input first so clicks don't bleed through to the world.
    // Each call is a no-op while that HUD is hidden.
    m_towerHUD.ProcessInput(game);
    m_scoreHUD.ProcessInput(game);
    m_towerInfoHUD.ProcessInput(game);
    HandleHudSignals(game);

    Vector2 mouseWorld = game.GetInput().GetWorldMousePosition(m_renderSystem.GetCamera());
    HandleTowerPlacement(game, mouseWorld);

    // Re-point the info panel once this frame's selection changes have settled
    SyncHUDState(game);
}

void PlayingState::SyncHUDState(Game& game) {
    if (m_selectedTowerKey != DenseSlotMap<Tower>::INVALID_KEY) {
        if (Tower* tower = game.GetGameData().towers.Get(m_selectedTowerKey)) {
            Vector2 screenPos = GetWorldToScreen2D(tower->m_position, m_renderSystem.GetCamera());
            m_towerInfoHUD.SetTarget(game, *tower, screenPos, !game.GetGameData().waveActive);
        } else {
            m_towerInfoHUD.Hide();
        }
        m_hoveredTower.reset();
        return;
    }

    // Nothing selected — show a preview panel for the hovered tower-bar button
    Vector2 mousePos = game.GetInput().GetMousePosition();
    const std::string& hovered = m_towerHUD.GetHoveredTower(mousePos);
    if (hovered.empty()) {
        m_hoveredTower.reset();
        m_towerInfoHUD.Hide();
        return;
    }

    // Only re-create the preview tower when the hovered type changes
    if (!m_hoveredTower.has_value() || m_hoveredTower->m_name != hovered)
        m_hoveredTower = game.GetTowerFactory().Create(hovered);
    Vector2 topCenter = m_towerHUD.GetHoveredButtonTopCenter(mousePos);
    m_towerInfoHUD.SetTarget(game, *m_hoveredTower, topCenter, false);
}

void PlayingState::HandleHudSignals(Game& game) {
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
        m_waveManager.StartWave(game.GetGameData());

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

    // Empty tile — always deselect the inspected tower
    m_selectedTowerKey = DenseSlotMap<Tower>::INVALID_KEY;

    // Placement is blocked during waves or when no tower type is selected
    if (game.GetGameData().waveActive) {
        if (!m_towerHUD.GetSelectedTower().empty())
            m_eventLog.Add("Cannot place towers during a wave!");
        return;
    }
    if (m_towerHUD.GetSelectedTower().empty()) return;
    int cost = game.GetTowerFactory().GetCost(m_towerHUD.GetSelectedTower());
    if (game.GetGameData().gold >= cost) {
        Tower tower = game.GetTowerFactory().Create(m_towerHUD.GetSelectedTower());
        if (m_worldSystem.PlaceTower(x, y, tower, game.GetGameData())) {
            game.GetGameData().gold -= cost;
            m_towerHUD.ClearSelection();
        }
    }
}


void PlayingState::Update(Game& game, float dt) {
    if (game.GetGameData().victory)
        game.ChangeState(std::make_unique<EndState>(true));

    if (m_gameOver)
        game.ChangeState(std::make_unique<EndState>(false));

    m_scoreHUD.SetAutoSpawn(m_waveManager.IsAutoSpawn());
    m_eventLog.Update(game, dt);

    m_waveManager.Update(dt, game.GetGameData(), m_worldSystem, game.GetEnemyFactory());

    m_enemySystem.TickEnemies(dt, game.GetGameData());
    m_enemySystem.FollowPath(dt, game.GetGameData());

    m_towerSystem.update(dt, game.GetGameData());

    m_worldSystem.CheckEnemyReachedCore(game.GetGameData());
    m_towerSystem.TickAttacks(dt, game.GetGameData());
    m_worldSystem.CheckEnemyDead(game.GetGameData(), game.GetEnemyFactory());
    m_worldSystem.CheckGameOver(m_gameOver, game.GetGameData());
}

void PlayingState::Draw(Game& game) {
    ClearBackground(DARKGRAY);

    BeginMode2D(m_renderSystem.GetCamera());
    m_renderSystem.DrawMap(game.GetGameData().map, game.GetAssets());
    m_renderSystem.DrawPaths(game.GetGameData().map);
    if (m_debug) {
        m_renderSystem.DebugDrawMap(game.GetGameData().map);
        m_renderSystem.DebugDrawEnemies(game.GetGameData().enemies);
    }
    m_renderSystem.DrawTowers(game.GetGameData().towers, game.GetAssets());

    if (m_selectedTowerKey != DenseSlotMap<Tower>::INVALID_KEY) {
        if (const Tower* t = game.GetGameData().towers.Get(m_selectedTowerKey))
            m_renderSystem.DrawTowerRange(t->m_position, t->m_radius, {255, 200, 50, 220});
    }

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

    // Draw order: info panel last so it sits on top. Hidden HUDs skip themselves.
    m_towerHUD.Draw(game);
    m_scoreHUD.Draw(game);
    m_eventLog.Draw(game);
    m_towerInfoHUD.Draw(game);
}
