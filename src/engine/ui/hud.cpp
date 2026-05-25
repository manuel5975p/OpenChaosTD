#include <engine/ui/hud.hpp>
#include <engine/core/input.hpp>
#include <algorithm>

void HUD::DrawPanelBackground(unsigned char alpha, bool border) const {
    DrawRectangleRec(m_panelRect, {20, 20, 20, alpha});
    if (border)
        DrawRectangleLinesEx(m_panelRect, 1.0f, {80, 80, 80, 255});
}

void HUD::ConsumePanelClick(Input& input) const {
    if (input.IsMousePressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointRec(input.GetMousePosition(), m_panelRect))
        input.ConsumeMouseInput();
}

void HUD::ClampPanelToScreen(int screenW, int screenH) {
    m_panelRect.x = std::clamp(m_panelRect.x, 0.0f,
                               static_cast<float>(screenW) - m_panelRect.width);
    m_panelRect.y = std::clamp(m_panelRect.y, 0.0f,
                               static_cast<float>(screenH) - m_panelRect.height);
}
