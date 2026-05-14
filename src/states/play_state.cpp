#include <states/play_state.hpp>

#include <states/end_state.hpp>
#include <game.hpp>
#include <raylib.h>


void PlayingState::OnEnter(Game& game) {
    game.GetGameData().Reset();

    m_worldSystem.GenerateMap(game.GetGameData().map, 15, 19);
    game.GetGameData().map.BuildPathMesh();

    m_renderSystem.CenterCamera(game.GetGameData().map, game.GetRenderer());

    m_towerHUD.Build(game);
}

void PlayingState::OnExit(Game& /*game*/) {}

void PlayingState::ProcessInput(Game& game, float dt) {
    // Toggle debug mode
    if(game.GetInput().IsPressed("Debug")) m_debug = !m_debug;

    // Control camera
    m_renderSystem.ControlCamera(dt, game.GetInput());

    m_towerHUD.ProcessInput(game);

    Vector2 mousePos = game.GetInput().GetMousePosition();

    // Place tower
    if (game.GetInput().IsPressed("PlaceTower") && !game.GetInput().IsMouseInputConsumed()) {
        int x, y;
        if (game.GetGameData().map.WorldToTile(GetScreenToWorld2D(mousePos, m_renderSystem.GetCamera()), x, y)) {
            int cost = game.GetTowerFactory().GetCost(m_towerHUD.GetSelectedTower());
            if (game.GetGameData().gold >= cost) {
                Tower tower = game.GetTowerFactory().Create(m_towerHUD.GetSelectedTower());
                if (m_worldSystem.PlaceTower(x, y, tower, game.GetGameData()))
                    game.GetGameData().gold -= cost;
            }
        }
    }

    // Remove tower
    if (game.GetInput().IsPressed("RemoveTower") && !game.GetInput().IsMouseInputConsumed()) {
        int x, y;
        if (game.GetGameData().map.WorldToTile(GetScreenToWorld2D(mousePos, m_renderSystem.GetCamera()), x, y))
            m_worldSystem.RemoveTower(x, y, game.GetGameData());
    }

    // Spawn enemies
    if(game.GetInput().IsPressed("Confirm")){
        m_worldSystem.SpawnEnemy(0, game.GetEnemyFactory().Create("voidno"), game.GetGameData());
        m_worldSystem.SpawnEnemy(1, game.GetEnemyFactory().Create("voidno"), game.GetGameData());
        m_worldSystem.SpawnEnemy(2, game.GetEnemyFactory().Create("voidno"), game.GetGameData());
    }
}

void PlayingState::Update(Game& game, float dt) {
    if (game.GetGameData().victory)
        game.ChangeState(std::make_unique<EndState>(true));

    if (m_gameOver)
        game.ChangeState(std::make_unique<EndState>(false));

    m_enemySystem.TickEnemies(dt, game.GetGameData());
    m_enemySystem.FollowPath(dt, game.GetGameData());

    m_towerSystem.update(dt, game.GetGameData());

    m_worldSystem.CheckEnemyReachedCore(game.GetGameData());
    m_worldSystem.TickAttacks(dt, game.GetGameData());
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

        for(auto& tower : game.GetGameData().towers){
            for(auto& key : tower.m_currentTargetKeys){
                const Enemy* enemy = game.GetGameData().enemies.Get(key);
                if (enemy) DrawCircleV(enemy->m_position, 4, RED);
            }
        }
    EndMode2D();

    m_towerHUD.Draw(game);

    DrawText(
        TextFormat("Lives: %d   Gold: %d   Score: %d",
                   game.GetGameData().lives, game.GetGameData().gold, game.GetGameData().score),
        20, 10, 18, RAYWHITE
    );
}

