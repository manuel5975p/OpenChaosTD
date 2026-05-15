#include <hud/hud.hpp>
#include <game.hpp>

void HUD::LoadScale(Game& game) {
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
