#include <hud/hud.hpp>
#include <engine/core/text.hpp>
#include <engine/core/input.hpp>
#include <algorithm>

void DrawTextCenteredX(const char* text, int centerX, int y, int fontSize, Color color, Text::Face face) {
    // Thin integer-coord forwarder to the shared float-coord helper.
    DrawCenteredText(text, static_cast<float>(centerX), static_cast<float>(y), fontSize, color, face);
}

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

bool HUD::BeginInput(Input& input, Vector2& mousePos, bool& pressed) {
    if (!m_visible) return false;
    mousePos = input.GetMousePosition();
    pressed = input.IsMousePressed(MOUSE_LEFT_BUTTON);
    ConsumePanelClick(input);
    return true;
}

bool HUD::BeginInput(Input& input) {
    if (!m_visible) return false;
    ConsumePanelClick(input);
    return true;
}
