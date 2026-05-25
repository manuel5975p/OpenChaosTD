#include <hud/tower_build_hud.hpp>
#include <game.hpp>
#include <raylib.h>

void TowerBuildHUD::Build(Game& game) {
    const auto& names = game.GetTowerFactory().GetNames();

    HUD::Build(game.GetGameConfig().hudScale);
    const float btnSize = Scaled(64.0f);
    const float panelH  = Scaled(80.0f);
    const float margin  = Scaled(8.0f);
    const float gap     = Scaled(4.0f);

    float y = game.GetScreen().GetGameHeight() - btnSize - margin;
    m_panelRect = { 0.0f, game.GetScreen().GetGameHeight() - panelH,
                    static_cast<float>(game.GetScreen().GetGameWidth()), panelH };

    m_buttons.clear();
    for (size_t i = 0; i < names.size(); i++) {
        Button btn;
        btn.m_label = names[i];
        btn.m_rect = { margin + i * (btnSize + gap), y, btnSize, btnSize };
        m_buttons.push_back(btn);
    }

    m_selectedTower = "";
}

void TowerBuildHUD::OnProcessInput(Game& game) {
    Vector2 mousePos = game.GetInput().GetMousePosition();

    ConsumePanelClick(game.GetInput());

    if (game.GetInput().IsMousePressed(MOUSE_LEFT_BUTTON)) {
        for (const Button& btn : m_buttons) {
            if (btn.IsClicked(mousePos, true)) {
                // Toggle: clicking the active type again clears the selection
                m_selectedTower = (m_selectedTower == btn.m_label) ? "" : btn.m_label;
                break;
            }
        }
    }
}

const std::string& TowerBuildHUD::GetHoveredTower(Vector2 mousePos) const {
    static const std::string empty;
    for (const auto& btn : m_buttons)
        if (CheckCollisionPointRec(mousePos, btn.m_rect))
            return btn.m_label;
    return empty;
}

Vector2 TowerBuildHUD::GetHoveredButtonTopCenter(Vector2 mousePos) const {
    for (const auto& btn : m_buttons)
        if (CheckCollisionPointRec(mousePos, btn.m_rect))
            return { btn.m_rect.x + btn.m_rect.width / 2.0f, btn.m_rect.y };
    return {};
}

void TowerBuildHUD::OnDraw(Game& game) {
    Vector2 mousePos = game.GetInput().GetMousePosition();

    DrawPanelBackground(200);

    for (const Button& btn : m_buttons) {
        const std::string& name = btn.m_label;
        bool selected = (name == m_selectedTower);

        btn.Draw(mousePos, selected);

        int fontSize = ScaledInt(8.0f);
        Texture2D& tex = game.GetResources().GetTexture(game.GetTowerFactory().GetTexture(name));
        float tw = static_cast<float>(tex.width);
        float th = static_cast<float>(tex.height);
        DrawTextureV(tex, { btn.m_rect.x + (btn.m_rect.width  - tw) / 2.0f,
                            btn.m_rect.y + (btn.m_rect.height - th) / 2.0f - Scaled(8.0f) }, WHITE);

        int centerX = static_cast<int>(btn.m_rect.x + btn.m_rect.width / 2.0f);
        DrawTextCenteredX(name.c_str(), centerX,
            static_cast<int>(btn.m_rect.y + btn.m_rect.height - Scaled(18.0f)),
            fontSize, LIGHTGRAY);

        int cost = game.GetTowerFactory().GetCost(name);
        const char* costStr = TextFormat("$%d", cost);
        Color costColor = (game.GetGameData().gold >= cost) ? GREEN : RED;
        DrawTextCenteredX(costStr, centerX,
            static_cast<int>(btn.m_rect.y + btn.m_rect.height - Scaled(9.0f)),
            fontSize, costColor);
    }
}
