#include <states/end_state.hpp>
#include <states/menu_state.hpp>
#include <states/play_state.hpp>
#include <game.hpp>
#include <raylib.h>

EndState::EndState(bool won) : m_won(won) {}

void EndState::OnEnter(Game& game) {
    float cx = game.GetRenderer().GetGameWidth()  / 2.0f;
    float cy = game.GetRenderer().GetGameHeight() / 2.0f;

    m_playAgainButton.m_label = "PLAY AGAIN";
    m_playAgainButton.m_rect = { cx - 90.0f, cy + 20.0f, 180.0f, 44.0f };

    m_menuButton.m_label = "MAIN MENU";
    m_menuButton.m_rect = { cx - 90.0f, cy + 74.0f, 180.0f, 44.0f };
}

void EndState::OnExit(Game& /*game*/) {}

void EndState::ProcessInput(Game& game, float /*dt*/) {
    Vector2 mouse = game.GetInput().GetMousePosition();
    bool clicked = game.GetInput().IsPressed("Select");

    if (m_playAgainButton.IsClicked(mouse, clicked))
        game.ChangeState(std::make_unique<PlayingState>());

    if (m_menuButton.IsClicked(mouse, clicked) || game.GetInput().IsPressed("Cancel"))
        game.ChangeState(std::make_unique<MenuState>());
}

void EndState::Update(Game& /*game*/, float /*dt*/) {}

void EndState::Draw(Game& game) {
    float cx = game.GetRenderer().GetGameWidth()  / 2.0f;
    float cy = game.GetRenderer().GetGameHeight() / 2.0f;
    Vector2 mouse = game.GetInput().GetMousePosition();

    ClearBackground(DARKGRAY);

    if (m_won) {
        const char* title = "VICTORY!";
        int tw = MeasureText(title, 48);
        DrawText(title, static_cast<int>(cx - tw / 2.0f), static_cast<int>(cy - 80), 48, GOLD);
    } else {
        const char* title = "GAME OVER";
        int tw = MeasureText(title, 48);
        DrawText(title, static_cast<int>(cx - tw / 2.0f), static_cast<int>(cy - 80), 48, RED);
    }

    const char* score = TextFormat("Score: %d", game.GetGameData().score);
    int sw = MeasureText(score, 24);
    DrawText(score, static_cast<int>(cx - sw / 2.0f), static_cast<int>(cy - 20), 24, RAYWHITE);

    m_playAgainButton.Draw(mouse);
    int paW = MeasureText(m_playAgainButton.m_label.c_str(), 20);
    DrawText(m_playAgainButton.m_label.c_str(),
        static_cast<int>(m_playAgainButton.m_rect.x + (m_playAgainButton.m_rect.width - paW) / 2.0f),
        static_cast<int>(m_playAgainButton.m_rect.y + (m_playAgainButton.m_rect.height - 20) / 2.0f),
        20, RAYWHITE);

    m_menuButton.Draw(mouse);
    int mW = MeasureText(m_menuButton.m_label.c_str(), 20);
    DrawText(m_menuButton.m_label.c_str(),
        static_cast<int>(m_menuButton.m_rect.x + (m_menuButton.m_rect.width - mW) / 2.0f),
        static_cast<int>(m_menuButton.m_rect.y + (m_menuButton.m_rect.height - 20) / 2.0f),
        20, RAYWHITE);
}
