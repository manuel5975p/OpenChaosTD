#include <states/play_state.hpp>

#include <states/menu_state.hpp>
#include <game.hpp>
#include <raylib.h>


void PlayingState::OnEnter(Game& game) {
    m_worldSystem.GenerateMap(game.GetGameData().map, 15, 19);

    // Center the map in the middle of the screen
    m_renderSystem.CenterCamera(game.GetGameData().map, game.GetRenderer());

    game.GetGameData().map.BuildPathMesh();

    game.GetGameData().lives = 20;

    Enemy enemy;
    enemy.m_speed = 50;
    enemy.m_health = 5;
    enemy.m_maxhealth = 10;
    m_worldSystem.SpawnEnemy(0, enemy, game.GetGameData());
    m_worldSystem.SpawnEnemy(1, enemy, game.GetGameData());
    m_worldSystem.SpawnEnemy(2, enemy, game.GetGameData());
}

void PlayingState::OnExit(Game& /*game*/) {

}

void PlayingState::ProcessInput(Game& game, float dt) {
    if(game.GetInput().IsPressed("Debug")) m_debug = !m_debug;

    m_renderSystem.ControlCamera(dt, game.GetInput());

    // Place tower
    int x, y;
    if(game.GetInput().IsMouseLeftPressed() && game.GetGameData().map.WorldToTile(GetScreenToWorld2D(game.GetInput().GetMousePosition(), m_renderSystem.GetCamera()), x, y)){
        Tower tower;
        m_worldSystem.PlaceTower(x, y, tower, game.GetGameData());
    }

    // Remove tower
    if(game.GetInput().IsMouseRightPressed() && game.GetGameData().map.WorldToTile(GetScreenToWorld2D(game.GetInput().GetMousePosition(), m_renderSystem.GetCamera()), x, y)){
         m_worldSystem.RemoveTower(x, y, game.GetGameData());
    }
}

void PlayingState::Update(Game& game, float dt) {
    if(m_gameOver){
        std::cout << "Game Over" <<  std::endl;
        game.ChangeState(std::make_unique<MenuState>());
    }

    m_worldSystem.UpdateEnemyPosition(dt, game.GetGameData());
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
    EndMode2D();

    DrawText("PLAYING - map renders here", 20, 20, 20, GREEN);
    DrawText(
        TextFormat("Lives: %d   Gold: %d   Score: %d",
                   game.GetGameData().lives, game.GetGameData().gold, game.GetGameData().score),
        20, game.GetRenderer().GetGameHeight() - 30, 18, RAYWHITE
    );
}