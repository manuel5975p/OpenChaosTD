#include <states/menu_state.hpp>
#include <game.hpp>
#include <raylib.h>
#include <states/datapack_select_state.hpp>
#include <states/settings_state.hpp>

void MenuState::OnEnter(Game& game) {
    // Returning to the menu is the single choke point that frees any active pack's
    // assets, templates and mounted resource path (idempotent if none is active).
    game.DeactivateDatapack();

    int cx = game.GetScreen().GetGameWidth()  / 2;
    int cy = game.GetScreen().GetGameHeight() / 2;
    m_playButton.m_label = "PLAY";
    m_playButton.m_rect = { static_cast<float>(cx - 80), static_cast<float>(cy + 20), 160.0f, 44.0f };

    m_settingsButton.m_label = "SETTINGS";
    m_settingsButton.m_rect = { static_cast<float>(cx - 80), static_cast<float>(cy + 74), 160.0f, 44.0f };

    m_particleEditorButton.m_label = "PARTICLE EDITOR";
    m_particleEditorButton.m_rect = { static_cast<float>(cx - 80), static_cast<float>(cy + 128), 160.0f, 44.0f };

    m_exitButton.m_label = "EXIT";
    m_exitButton.m_rect = { static_cast<float>(cx - 80), static_cast<float>(cy + 182), 160.0f, 44.0f };
}

void MenuState::OnExit(Game& /*game*/) {

}

void MenuState::ProcessInput(Game& game, float /*dt*/) {
    Vector2 mousePos = game.GetInput().GetMousePosition();
    bool clicked = game.GetInput().IsMousePressed(MOUSE_LEFT_BUTTON);

    m_playButton.Update(mousePos, clicked);
    m_settingsButton.Update(mousePos, clicked);
    m_particleEditorButton.Update(mousePos, clicked);
    m_exitButton.Update(mousePos, clicked);

    // Play and the particle editor both need an active datapack, so they route
    // through the selection screen first (carrying where to go once a pack is chosen).
    if (m_playButton.IsClicked() || game.GetInput().IsPressed("Confirm"))
        game.ChangeState(std::make_unique<DatapackSelectState>(DatapackSelectState::Intent::Play));

    if (m_settingsButton.IsClicked())
        game.ChangeState(std::make_unique<SettingsState>());

    if (m_particleEditorButton.IsClicked())
        game.ChangeState(std::make_unique<DatapackSelectState>(DatapackSelectState::Intent::EditParticles));

    if (m_exitButton.IsClicked() || game.GetInput().IsPressed("Cancel"))
        game.Quit();
}

void MenuState::Update(Game& /*game*/, float /*dt*/) {}

void MenuState::Draw(Game& game){
    const int cx = game.GetScreen().GetGameWidth()  / 2;
    const int cy = game.GetScreen().GetGameHeight() / 2;

    ClearBackground(DARKGRAY);
    DrawText("OPEN CHAOS TD", cx - 180, cy - 80, 40, RAYWHITE);

    m_playButton.Draw();
    m_playButton.DrawLabel(24, RAYWHITE);

    m_settingsButton.Draw();
    m_settingsButton.DrawLabel(24, RAYWHITE);

    m_particleEditorButton.Draw();
    m_particleEditorButton.DrawLabel(16, RAYWHITE);

    m_exitButton.Draw();
    m_exitButton.DrawLabel(24, RAYWHITE);
}