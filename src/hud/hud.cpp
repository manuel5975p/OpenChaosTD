#include <hud/hud.hpp>
#include <game.hpp>
#include <algorithm>

void DrawTextCenteredX(const char* text, int centerX, int y, int fontSize, Color color) {
    int width = MeasureText(text, fontSize);
    DrawText(text, centerX - width / 2, y, fontSize, color);
}

void HUD::Build(Game& game) {
    m_scale = game.GetGameConfig().hudScale;
}

void HUD::DrawPanelBackground(unsigned char alpha, bool border) const {
    DrawRectangleRec(m_panelRect, {20, 20, 20, alpha});
    if (border)
        DrawRectangleLinesEx(m_panelRect, 1.0f, {80, 80, 80, 255});
}

void HUD::ConsumePanelClick(Game& game, const char* action) const {
    if (game.GetInput().IsPressed(action) &&
        CheckCollisionPointRec(game.GetInput().GetMousePosition(), m_panelRect))
        game.GetInput().ConsumeMouseInput();
}

void HUD::ClampPanelToScreen(int screenW, int screenH) {
    m_panelRect.x = std::clamp(m_panelRect.x, 0.0f,
                               static_cast<float>(screenW) - m_panelRect.width);
    m_panelRect.y = std::clamp(m_panelRect.y, 0.0f,
                               static_cast<float>(screenH) - m_panelRect.height);
}
