#include <hud/pause_hud.hpp>
#include <engine/core/input.hpp>
#include <raylib.h>

void PauseHUD::Build(float scale, int screenW, int screenH) {
    HUD::Build(scale);
    Hide(); // shown only while the game is paused

    m_screenW = screenW;
    m_screenH = screenH;
    float gw = static_cast<float>(screenW);
    float gh = static_cast<float>(screenH);

    // Centered panel sized to hold the title and five stacked buttons.
    float panelW = Scaled(240.0f);
    float panelH = Scaled(400.0f);
    m_panelRect = { (gw - panelW) / 2.0f, (gh - panelH) / 2.0f, panelW, panelH };

    float btnW = Scaled(180.0f);
    float btnH = Scaled(44.0f);
    float btnX = (gw - btnW) / 2.0f;
    float spacing = Scaled(56.0f);
    float firstY = m_panelRect.y + Scaled(80.0f);

    m_resumeBtn.m_label = "RESUME";
    m_resumeBtn.m_rect = { btnX, firstY, btnW, btnH };
    m_saveBtn.m_label = "SAVE";
    m_saveBtn.m_rect = { btnX, firstY + spacing, btnW, btnH };
    m_loadBtn.m_label = "LOAD";
    m_loadBtn.m_rect = { btnX, firstY + spacing * 2.0f, btnW, btnH };
    m_restartBtn.m_label = "RESTART";
    m_restartBtn.m_rect = { btnX, firstY + spacing * 3.0f, btnW, btnH };
    m_mainMenuBtn.m_label = "MAIN MENU";
    m_mainMenuBtn.m_rect = { btnX, firstY + spacing * 4.0f, btnW, btnH };
}

void PauseHUD::ProcessInput(Input& input) {
    if (!m_visible) return;

    Vector2 mousePos = input.GetMousePosition();
    bool pressed = input.IsMousePressed(MOUSE_LEFT_BUTTON);

    // Swallow clicks landing on the panel so they never reach the game grid behind it.
    ConsumePanelClick(input);

    m_resumeBtn.Update(mousePos, pressed);
    m_saveBtn.Update(mousePos, pressed);
    m_loadBtn.Update(mousePos, pressed);
    m_restartBtn.Update(mousePos, pressed);
    m_mainMenuBtn.Update(mousePos, pressed);

    if (m_resumeBtn.IsClicked())   m_resumeSignal.Raise();
    if (m_saveBtn.IsClicked())     m_saveSignal.Raise();
    if (m_loadBtn.IsClicked())     m_loadSignal.Raise();
    if (m_restartBtn.IsClicked())  m_restartSignal.Raise();
    if (m_mainMenuBtn.IsClicked()) m_mainMenuSignal.Raise();
}

void PauseHUD::Draw() {
    if (!m_visible) return;

    // Dim the whole screen so the map, towers, and enemies stay visible behind the menu.
    DrawRectangle(0, 0, m_screenW, m_screenH, {0, 0, 0, 120});

    // Panel background reuses the shared HUD style (dark fill + subtle border).
    DrawPanelBackground(230, true);

    int centerX = static_cast<int>(m_panelRect.x + m_panelRect.width / 2.0f);
    int titleY = static_cast<int>(m_panelRect.y + Scaled(28.0f));
    DrawTextCenteredX("PAUSED", centerX, titleY, ScaledInt(28.0f), RAYWHITE);

    int fontBtn = ScaledInt(18.0f);
    m_resumeBtn.Draw();
    m_resumeBtn.DrawLabel(fontBtn, RAYWHITE);
    m_saveBtn.Draw();
    m_saveBtn.DrawLabel(fontBtn, RAYWHITE);
    m_loadBtn.Draw();
    m_loadBtn.DrawLabel(fontBtn, RAYWHITE);
    m_restartBtn.Draw();
    m_restartBtn.DrawLabel(fontBtn, RAYWHITE);
    m_mainMenuBtn.Draw();
    m_mainMenuBtn.DrawLabel(fontBtn, RAYWHITE);
}
