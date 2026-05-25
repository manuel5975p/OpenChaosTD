#include <states/menu_state.hpp>
#include <game.hpp>
#include <raylib.h>
#include <states/play_state.hpp>

void MenuState::OnEnter(Game& game) {
    int cx = game.GetScreen().GetGameWidth()  / 2;
    int cy = game.GetScreen().GetGameHeight() / 2;
    m_playButton.m_label = "PLAY";
    m_playButton.m_rect = { static_cast<float>(cx - 80), static_cast<float>(cy + 20), 160.0f, 44.0f };

    m_exitButton.m_label = "EXIT";
    m_exitButton.m_rect = { static_cast<float>(cx - 80), static_cast<float>(cy + 74), 160.0f, 44.0f };
}

void MenuState::OnExit(Game& /*game*/) {

}

void MenuState::ProcessInput(Game& game, float /*dt*/) {
    Vector2 mousePos = game.GetInput().GetMousePosition();
    bool clicked = game.GetInput().IsPressed("Select");

    if (m_playButton.IsClicked(mousePos, clicked) || game.GetInput().IsPressed("Confirm"))
        game.ChangeState(std::make_unique<PlayingState>());

    if (m_exitButton.IsClicked(mousePos, clicked) || game.GetInput().IsPressed("Cancel"))
        game.Quit();
}

void MenuState::Update(Game& /*game*/, float /*dt*/) {}

void MenuState::Draw(Game& game){
    const int cx = game.GetScreen().GetGameWidth()  / 2;
    const int cy = game.GetScreen().GetGameHeight() / 2;

    ClearBackground(DARKGRAY);
    DrawText("OPEN CHAOS TD", cx - 180, cy - 80, 40, RAYWHITE);

    Vector2 mousePos = game.GetInput().GetMousePosition();
    auto drawButton = [&](const Button& btn) {
        btn.Draw(mousePos);
        const char* label = btn.m_label.c_str();
        int lw = MeasureText(label, 24);
        DrawText(label,
            static_cast<int>(btn.m_rect.x + (btn.m_rect.width - lw) / 2.0f),
            static_cast<int>(btn.m_rect.y + (btn.m_rect.height - 24) / 2.0f),
            24, RAYWHITE);
    };

    drawButton(m_playButton);
    drawButton(m_exitButton);
}