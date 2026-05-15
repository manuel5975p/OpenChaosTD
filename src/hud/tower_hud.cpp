#include <hud/tower_hud.hpp>
#include <game.hpp>
#include <raylib.h>

void TowerHUD::Build(Game& game) {
    const auto& names = game.GetTowerFactory().GetNames();

    LoadScale(game);
    const float btnSize = Scaled(64.0f);
    const float panelH  = Scaled(80.0f);
    const float margin  = Scaled(8.0f);
    const float gap     = Scaled(4.0f);
    m_btnSize = btnSize;

    float y = game.GetRenderer().GetGameHeight() - btnSize - margin;
    m_panelRect = { 0.0f, game.GetRenderer().GetGameHeight() - panelH,
                    static_cast<float>(game.GetRenderer().GetGameWidth()), panelH };

    m_buttons.clear();
    for (size_t i = 0; i < names.size(); i++) {
        Button btn;
        btn.m_label = names[i];
        btn.m_rect = { margin + i * (btnSize + gap), y, btnSize, btnSize };
        m_buttons.push_back(btn);
    }

    m_selectedTower = "";
}

void TowerHUD::ProcessInput(Game& game) {
    Vector2 mousePos = game.GetInput().GetMousePosition();

    ConsumePanelClick(game, "PlaceTower");

    if (game.GetInput().IsPressed("PlaceTower")) {
        const auto& names = game.GetTowerFactory().GetNames();
        for (size_t i = 0; i < m_buttons.size(); i++) {
            if (m_buttons[i].IsClicked(mousePos, true)) {
                // Toggle: clicking the active type again clears the selection
                m_selectedTower = (m_selectedTower == names[i]) ? "" : names[i];
                break;
            }
        }
    }
}

const std::string& TowerHUD::GetHoveredTower(Vector2 mousePos) const {
    static const std::string empty;
    for (const auto& btn : m_buttons)
        if (CheckCollisionPointRec(mousePos, btn.m_rect))
            return btn.m_label;
    return empty;
}

Vector2 TowerHUD::GetHoveredButtonTopCenter(Vector2 mousePos) const {
    for (const auto& btn : m_buttons)
        if (CheckCollisionPointRec(mousePos, btn.m_rect))
            return { btn.m_rect.x + btn.m_rect.width / 2.0f, btn.m_rect.y };
    return {};
}

void TowerHUD::Draw(Game& game) {
    const auto& names = game.GetTowerFactory().GetNames();
    Vector2 mousePos = game.GetInput().GetMousePosition();

    DrawPanelBackground(200);

    for (size_t i = 0; i < m_buttons.size(); i++) {
        const std::string& name = names[i];
        const Button& btn = m_buttons[i];
        bool selected = (name == m_selectedTower);

        btn.Draw(mousePos, selected);

        int fontSize = ScaledInt(8.0f);
        Texture2D& tex = game.GetAssets().GetTexture(game.GetTowerFactory().GetTexture(name));
        float tw = static_cast<float>(tex.width);
        float th = static_cast<float>(tex.height);
        DrawTextureV(tex, { btn.m_rect.x + (btn.m_rect.width  - tw) / 2.0f,
                            btn.m_rect.y + (btn.m_rect.height - th) / 2.0f - Scaled(8.0f) }, WHITE);

        int nameW = MeasureText(name.c_str(), fontSize);
        DrawText(name.c_str(),
            static_cast<int>(btn.m_rect.x + (btn.m_rect.width - nameW) / 2.0f),
            static_cast<int>(btn.m_rect.y + btn.m_rect.height - Scaled(18.0f)),
            fontSize, LIGHTGRAY);

        int cost = game.GetTowerFactory().GetCost(name);
        const char* costStr = TextFormat("$%d", cost);
        int costW = MeasureText(costStr, fontSize);
        Color costColor = (game.GetGameData().gold >= cost) ? GREEN : RED;
        DrawText(costStr,
            static_cast<int>(btn.m_rect.x + (btn.m_rect.width - costW) / 2.0f),
            static_cast<int>(btn.m_rect.y + btn.m_rect.height - Scaled(9.0f)),
            fontSize, costColor);
    }
}
