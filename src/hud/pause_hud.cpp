#include <hud/pause_hud.hpp>
#include <game.hpp>
#include <raylib.h>

void PauseHUD::Build(Game& game) {
    HUD::Build(game.GetGameConfig().hudScale);
    Hide(); // shown only while the game is paused

    float gw = static_cast<float>(game.GetScreen().GetGameWidth());
    float gh = static_cast<float>(game.GetScreen().GetGameHeight());

    // Centered panel sized to hold the title and three stacked buttons.
    float panelW = Scaled(240.0f);
    float panelH = Scaled(260.0f);
    m_panelRect = { (gw - panelW) / 2.0f, (gh - panelH) / 2.0f, panelW, panelH };

    float btnW = Scaled(180.0f);
    float btnH = Scaled(44.0f);
    float btnX = (gw - btnW) / 2.0f;
    float spacing = Scaled(56.0f);
    float firstY = m_panelRect.y + Scaled(88.0f);

    m_resumeBtn.m_label = "RESUME";
    m_resumeBtn.m_rect = { btnX, firstY, btnW, btnH };
    m_restartBtn.m_label = "RESTART";
    m_restartBtn.m_rect = { btnX, firstY + spacing, btnW, btnH };
    m_mainMenuBtn.m_label = "MAIN MENU";
    m_mainMenuBtn.m_rect = { btnX, firstY + spacing * 2.0f, btnW, btnH };
}

void PauseHUD::OnProcessInput(Game& game) {
    Vector2 mousePos = game.GetInput().GetMousePosition();
    bool pressed = game.GetInput().IsMousePressed(MOUSE_LEFT_BUTTON);

    // Swallow clicks landing on the panel so they never reach the game grid behind it.
    ConsumePanelClick(game.GetInput());

    m_resumeBtn.Update(mousePos, pressed);
    m_restartBtn.Update(mousePos, pressed);
    m_mainMenuBtn.Update(mousePos, pressed);

    if (m_resumeBtn.IsClicked())   m_resumeSignal.Raise();
    if (m_restartBtn.IsClicked())  m_restartSignal.Raise();
    if (m_mainMenuBtn.IsClicked()) m_mainMenuSignal.Raise();
}

void PauseHUD::OnDraw(Game& game) {
    int gw = game.GetScreen().GetGameWidth();
    int gh = game.GetScreen().GetGameHeight();

    // Dim the whole screen so the map, towers, and enemies stay visible behind the menu.
    DrawRectangle(0, 0, gw, gh, {0, 0, 0, 120});

    // Panel background reuses the shared HUD style (dark fill + subtle border).
    DrawPanelBackground(230, true);

    int centerX = static_cast<int>(m_panelRect.x + m_panelRect.width / 2.0f);
    int titleY = static_cast<int>(m_panelRect.y + Scaled(28.0f));
    DrawTextCenteredX("PAUSED", centerX, titleY, ScaledInt(28.0f), RAYWHITE);

    int fontBtn = ScaledInt(18.0f);
    m_resumeBtn.Draw();
    m_resumeBtn.DrawLabel(fontBtn, RAYWHITE);
    m_restartBtn.Draw();
    m_restartBtn.DrawLabel(fontBtn, RAYWHITE);
    m_mainMenuBtn.Draw();
    m_mainMenuBtn.DrawLabel(fontBtn, RAYWHITE);
}
