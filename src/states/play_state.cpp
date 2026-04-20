#include <states/play_state.hpp>

#include <states/game_over_state.hpp>
#include <game.hpp>
#include <raylib.h>


void PlayingState::OnEnter(Game& game) {
    // Reset GameData
    game.GetGameData().Reset();

    m_worldSystem.GenerateMap(game.GetGameData().map, 15, 19);
    game.GetGameData().map.BuildPathMesh();

    // Center the map in the middle of the screen
    m_renderSystem.CenterCamera(game.GetGameData().map, game.GetRenderer());

}

void PlayingState::OnExit(Game& game) {
    
}

void PlayingState::ProcessInput(Game& game, float dt) {
    // Toggle debug mode
    if(game.GetInput().IsPressed("Debug")) m_debug = !m_debug;

    // Control camera
    m_renderSystem.ControlCamera(dt, game.GetInput());

    // Place tower
    int x, y;
    if(game.GetInput().IsMouseLeftPressed() && game.GetGameData().map.WorldToTile(GetScreenToWorld2D(game.GetInput().GetMousePosition(), m_renderSystem.GetCamera()), x, y)){
        Tower slowTower;
        slowTower.m_targetCount = 0;
        slowTower.m_fireRate = 1;
        slowTower.m_radius = 128;
        slowTower.m_targetingMode = TargetingMode::First;
        slowTower.AddModule(std::make_unique<SlowModule>(0.6f, 2.0f));
        m_worldSystem.PlaceTower(x, y, slowTower, game.GetGameData());
    }

    // Remove tower
    if(game.GetInput().IsMouseRightPressed() && game.GetGameData().map.WorldToTile(GetScreenToWorld2D(game.GetInput().GetMousePosition(), m_renderSystem.GetCamera()), x, y)){
         m_worldSystem.RemoveTower(x, y, game.GetGameData());
    }

    // Spawn enemies
    if(game.GetInput().IsPressed("Confirm")){
        Enemy enemy;
        enemy.m_speed = 50;
        enemy.m_health = 2;
        enemy.m_maxhealth = 10;
        m_worldSystem.SpawnEnemy(0, enemy, game.GetGameData());
        m_worldSystem.SpawnEnemy(1, enemy, game.GetGameData());
        m_worldSystem.SpawnEnemy(2, enemy, game.GetGameData());
    }
}

void PlayingState::Update(Game& game, float dt) {
    if(m_gameOver){
        std::cout << "Game Over" <<  std::endl;
        game.ChangeState(std::make_unique<GameOverState>());
    }

    m_enemySystem.UpdateEnemyPosition(dt, game.GetGameData());

    m_worldSystem.CheckEnemyReachedCore(game.GetGameData());
    m_worldSystem.CheckGameOver(m_gameOver, game.GetGameData());
}

void PlayingState::Draw(Game& game) {
    ClearBackground(DARKGRAY);

    BeginMode2D(m_renderSystem.GetCamera());
        m_renderSystem.DrawMap(game.GetGameData().map, game.GetAssets());
        m_renderSystem.DrawPaths(game.GetGameData().map);
        if(m_debug)
            m_renderSystem.DebugDrawMap(game.GetGameData().map);
        m_renderSystem.DrawTowers(game.GetGameData().towers, game.GetAssets());

        m_renderSystem.DrawEnemies(game.GetGameData().enemies, game.GetAssets());

        for(auto& tower : game.GetGameData().towers){
            std::vector<DenseSlotMap<Enemy>::Key> keys = m_worldSystem.SelectTargets(tower, game.GetGameData().enemies, tower.m_targetCount);
            for(auto& key : keys){
                Enemy enemy = *game.GetGameData().enemies.Get(key);
                DrawCircleV(enemy.m_position, 4, RED);
            }
            
        }
    EndMode2D();

    DrawText(
        TextFormat("Lives: %d   Gold: %d   Score: %d",
                   game.GetGameData().lives, game.GetGameData().gold, game.GetGameData().score),
        20, game.GetRenderer().GetGameHeight() - 30, 18, RAYWHITE
    );
}