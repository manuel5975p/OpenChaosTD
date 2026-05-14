#include <hud/tower_hud.hpp>
#include <game.hpp>
#include <raylib.h>

void TowerHUD::Build(Game& game) {
    const auto& names = game.GetTowerFactory().GetNames();

    constexpr float btnSize = 64.0f;
    constexpr float panelH = 80.0f;
    constexpr float margin = 8.0f;
    constexpr float gap = 4.0f;
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

    if (!names.empty())
        m_selectedTower = names[0];
}

void TowerHUD::ProcessInput(Game& game) {
    Vector2 mousePos = game.GetInput().GetMousePosition();
    bool anyPressed = game.GetInput().IsPressed("PlaceTower") || game.GetInput().IsPressed("RemoveTower");

    if (anyPressed && CheckCollisionPointRec(mousePos, m_panelRect))
        game.GetInput().ConsumeMouseInput();

    if (game.GetInput().IsPressed("PlaceTower")) {
        const auto& names = game.GetTowerFactory().GetNames();
        for (size_t i = 0; i < m_buttons.size(); i++) {
            if (m_buttons[i].IsClicked(mousePos, true)) {
                m_selectedTower = names[i];
                break;
            }
        }
    }
}

void TowerHUD::Draw(Game& game) {
    const auto& names = game.GetTowerFactory().GetNames();
    Vector2 mousePos = game.GetInput().GetMousePosition();

    DrawRectangleRec(m_panelRect, {20, 20, 20, 200});

    for (size_t i = 0; i < m_buttons.size(); i++) {
        const std::string& name = names[i];
        const Button& btn = m_buttons[i];
        bool selected = (name == m_selectedTower);

        btn.Draw(mousePos, selected);

        Texture2D& tex = game.GetAssets().GetTexture(game.GetTowerFactory().GetTexture(name));
        float tw = static_cast<float>(tex.width);
        float th = static_cast<float>(tex.height);
        DrawTextureV(tex, { btn.m_rect.x + (btn.m_rect.width  - tw) / 2.0f,
                            btn.m_rect.y + (btn.m_rect.height - th) / 2.0f - 8.0f }, WHITE);

        int nameW = MeasureText(name.c_str(), 8);
        DrawText(name.c_str(),
            static_cast<int>(btn.m_rect.x + (btn.m_rect.width - nameW) / 2.0f),
            static_cast<int>(btn.m_rect.y + btn.m_rect.height - 18),
            8, LIGHTGRAY);

        int cost = game.GetTowerFactory().GetCost(name);
        const char* costStr = TextFormat("$%d", cost);
        int costW = MeasureText(costStr, 8);
        Color costColor = (game.GetGameData().gold >= cost) ? GREEN : RED;
        DrawText(costStr,
            static_cast<int>(btn.m_rect.x + (btn.m_rect.width - costW) / 2.0f),
            static_cast<int>(btn.m_rect.y + btn.m_rect.height - 9),
            8, costColor);
    }
}
