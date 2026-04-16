#include <states/game_over_state.hpp>

#include <states/menu_state.hpp>
#include <states/play_state.hpp>
#include <game.hpp>
#include <raylib.h>

void GameOverState::OnEnter(Game& /*game*/) {}
void GameOverState::OnExit(Game& /*game*/) {}

void GameOverState::ProcessInput(Game& game, float /*dt*/) {
    if (game.GetInput().IsPressed("Confirm"))
        game.ChangeState(std::make_unique<MenuState>());
}

void GameOverState::Update(Game& /*game*/, float /*dt*/) {}

void GameOverState::Draw(Game& game) {
    const int cx = game.GetRenderer().GetGameWidth()  / 2;
    const int cy = game.GetRenderer().GetGameHeight() / 2;

    ClearBackground(DARKGRAY);
    DrawText("GAME OVER",           cx - 140, cy - 80, 48, RED);
    DrawText("Press ENTER for Menu", cx - 155, cy,     24, LIGHTGRAY);

    DrawText(
        TextFormat("Score: %d", game.GetGameData().score),
        cx - 60, cy + 40, 24, RAYWHITE
    );
}