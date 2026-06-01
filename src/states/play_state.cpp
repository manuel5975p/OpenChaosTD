#include <states/play_state.hpp>

#include <states/end_state.hpp>
#include <game.hpp>
#include <raylib.h>
#include <raymath.h>

// --- Base overrides --------------------------------------------------------

void PlayingState::OnEnter(Game& game) {
    game.GetGameData().Reset();
    game.GetParticles().Clear();
    game.GetSoundSystem().PlayMusic("openchaostd_main");

    m_mapGenerator.Generate(game.GetGameData().map, 15, 19, 3, 40);

    m_renderSystem.CenterCamera(game.GetGameData().map, game.GetScreen());

    m_towerHUD.Build(game);
    m_scoreHUD.Build(game);
    m_towerInfoHUD.Build(game);
    m_eventLog.Build(game.GetGameConfig().hudScale);

    m_waveManager.Load(game.GetJsonStore());
}

void PlayingState::OnExit(Game& game) {
    game.GetSoundSystem().StopMusic();
}

void PlayingState::ProcessInput(Game& game, float dt) {
    if (game.GetInput().IsPressed("Debug")) m_debug = !m_debug;
    if (game.GetInput().IsPressed("Speed")) CycleSpeed();
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

void PlayingState::Update(Game& game, float dt) {
    if (game.GetGameData().victory)
        game.ChangeState(std::make_unique<EndState>(true));

    if (m_gameOver)
        game.ChangeState(std::make_unique<EndState>(false));

    m_scoreHUD.SetAutoSpawn(m_waveManager.IsAutoSpawn());
    m_scoreHUD.SetSpeed(kSpeedSteps[m_speedIndex]);
    m_eventLog.Update(game, dt); // HUD fade tracks real time, not game speed

    // Run the simulation once per speed step. Each sub-step uses the real frame dt so per-frame
    // timing (tower cooldowns, spawn pacing) stays accurate instead of feeding one oversized step.
    for (int s = 0; s < kSpeedSteps[m_speedIndex] && !m_gameOver && !game.GetGameData().victory; s++)
        StepSimulation(game, dt);
}

void PlayingState::StepSimulation(Game& game, float dt) {
    m_waveManager.Update(dt, game.GetGameData(), m_worldSystem, game.GetEnemyFactory());

    m_enemySystem.TickEnemies(dt, game.GetGameData(), game.GetParticles());
    m_enemySystem.FollowPath(dt, game.GetGameData());

    m_towerSystem.Update(dt, game.GetGameData(), game.GetParticles());
    m_towerSystem.TickPayloads(game.GetGameData(), game.GetParticles());
    m_towerSystem.TickVfx(dt, game.GetGameData());
    game.GetParticles().Tick(dt);

    m_worldSystem.CheckEnemyReachedCore(game.GetGameData());
    m_worldSystem.CheckEnemyDead(game.GetGameData(), game.GetEnemyFactory(), game.GetParticles());
    m_worldSystem.CheckGameOver(m_gameOver, game.GetGameData());
}

void PlayingState::Draw(Game& game) {
    ClearBackground(DARKGRAY);
    Vector2 mouseWorld = game.GetInput().GetWorldMousePosition(m_renderSystem.GetCamera());

    BeginMode2D(m_renderSystem.GetCamera());
    m_renderSystem.DrawMap(game.GetGameData().map, game.GetResources());
    m_renderSystem.DrawPaths(game.GetGameData().map);
    if (m_debug) {
        m_renderSystem.DebugDrawMap(game.GetGameData().map);
        m_renderSystem.DebugDrawEnemies(game.GetGameData().enemies);
    }
    m_renderSystem.DrawTowers(game.GetGameData().towers, game.GetResources());
    m_renderSystem.DrawRangeIndicator(m_selection.towerKey, game.GetGameData().map, game.GetGameData().towers, mouseWorld);
    if (!game.GetGameData().waveActive &&
        m_selection.towerKey == DenseSlotMap<Tower>::INVALID_KEY &&
        !m_towerHUD.GetSelectedTower().empty()) {
        int x, y;
        if (game.GetGameData().map.WorldToTile(mouseWorld, x, y)) {
            const std::string& name = m_towerHUD.GetSelectedTower();
            float half = game.GetGameData().map.GetTileSize() / 2.0f;
            Vector2 tileCenter = Vector2Add(game.GetGameData().map.TileToWorld(x, y), {half, half});
            Texture2D& tex = game.GetResources().GetTexture(game.GetTowerFactory().GetTexture(name));
            m_renderSystem.DrawGhostTower(tileCenter, game.GetTowerFactory().GetRange(name), tex);
        }
    }
    m_renderSystem.DrawEnemies(game.GetGameData().enemies, game.GetResources());
    m_renderSystem.DrawVfx(game.GetGameData().m_vfx);
    game.GetParticles().Draw();
    EndMode2D();

    // Draw order: info panel last so it sits on top. Hidden HUDs skip themselves.
    m_towerHUD.Draw(game);
    m_scoreHUD.Draw(game);
    m_eventLog.Draw(game);
    m_towerInfoHUD.Draw(game);
}

// --- Input helpers ---------------------------------------------------------

void PlayingState::HandleHudSignals(Game& game) {
    if (m_towerInfoHUD.WasSellRequested()) {
        if (Tower* tower = game.GetGameData().towers.Get(m_selection.towerKey)) {
            int x, y;
            if (game.GetGameData().map.WorldToTile(tower->m_position, x, y)) {
                game.GetGameData().gold += static_cast<int>(tower->m_cost * game.GetGameData().sellRefundRate);
                m_worldSystem.RemoveTower(x, y, game.GetGameData());
            }
        }
        m_selection.towerKey = DenseSlotMap<Tower>::INVALID_KEY;
    }

    if (m_towerInfoHUD.WasTargetingCycleRequested())
        if (Tower* tower = game.GetGameData().towers.Get(m_selection.towerKey))
            tower->m_base.targetingMode = NextTargetingMode(tower->m_base.targetingMode);

    if (m_towerInfoHUD.WasUpgradeRequested())
        UpgradeSelectedTower(game);

    if (m_scoreHUD.WasWaveRequested())
        m_waveManager.StartWave(game.GetGameData());

    if (m_scoreHUD.WasAutoToggled())
        m_waveManager.ToggleAutoSpawn();

    if (m_scoreHUD.WasSpeedToggled())
        CycleSpeed();
}

void PlayingState::UpgradeSelectedTower(Game& game) {
    Tower* tower = game.GetGameData().towers.Get(m_selection.towerKey);
    if (!tower || !tower->m_upgrades) return;

    const auto& ups = *tower->m_upgrades;
    if (tower->m_level >= static_cast<int>(ups.size())) return; // already max level

    const TowerUpgrade& up = ups[tower->m_level];
    if (game.GetGameData().gold < up.cost) return;

    game.GetGameData().gold -= up.cost;
    for (auto& [k, v] : up.adds) {
        ApplyStat(tower->m_base, k, v, false);
        for (auto& mod : tower->m_modules) mod->PatchStat(k, v, false);
    }
    for (auto& [k, v] : up.muls) {
        ApplyStat(tower->m_base, k, v, true);
        for (auto& mod : tower->m_modules) mod->PatchStat(k, v, true);
    }
    for (auto& mod : up.addModules)
        if (auto m = game.GetTowerFactory().BuildModule(mod)) tower->AddModule(std::move(m));

    tower->m_cost += up.cost; // sell refund reflects total invested
    tower->m_level++;
}

void PlayingState::HandleTowerPlacement(Game& game, Vector2 mouseWorld) {
    if (!game.GetInput().IsMousePressed(MOUSE_LEFT_BUTTON)) return;
    if (game.GetInput().IsMouseInputConsumed()) return;

    int x, y;
    if (!game.GetGameData().map.WorldToTile(mouseWorld, x, y)) {
        // Clicked outside the map — deselect
        m_selection.towerKey = DenseSlotMap<Tower>::INVALID_KEY;
        return;
    }

    Tile& tile = game.GetGameData().map.Get(x, y);

    // Tile has a tower — select it regardless of wave state
    if (tile.m_towerKey != DenseSlotMap<Tower>::INVALID_KEY) {
        m_selection.towerKey = tile.m_towerKey;
        return;
    }

    // Empty tile — always deselect the inspected tower
    m_selection.towerKey = DenseSlotMap<Tower>::INVALID_KEY;

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

void PlayingState::SyncHUDState(Game& game) {
    if (m_selection.towerKey != DenseSlotMap<Tower>::INVALID_KEY) {
        if (Tower* tower = game.GetGameData().towers.Get(m_selection.towerKey)) {
            Vector2 screenPos = GetWorldToScreen2D(tower->m_position, m_renderSystem.GetCamera());
            m_towerInfoHUD.SetTarget(game, *tower, screenPos, true);
        } else {
            m_towerInfoHUD.Hide();
        }
        m_selection.hoveredTowerName.clear();
        return;
    }

    // Nothing selected — show a preview panel for the hovered tower-bar button
    Vector2 mousePos = game.GetInput().GetMousePosition();
    const std::string& hovered = m_towerHUD.GetHoveredTower(mousePos);
    if (hovered.empty()) {
        m_selection.hoveredTowerName.clear();
        m_towerInfoHUD.Hide();
        return;
    }

    // Only rebuild the preview tower when the hovered type changes
    if (m_selection.hoveredTowerName != hovered) {
        m_selection.hoveredTowerName = hovered;
        m_hoveredTowerCache = game.GetTowerFactory().Create(hovered);
    }
    Vector2 topCenter = m_towerHUD.GetHoveredButtonTopCenter(mousePos);
    m_towerInfoHUD.SetTarget(game, m_hoveredTowerCache, topCenter, false);
}
