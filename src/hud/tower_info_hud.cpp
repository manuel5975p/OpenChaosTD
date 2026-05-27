#include <hud/tower_info_hud.hpp>
#include <game.hpp>
#include <raylib.h>
#include <sstream>

void TowerInfoHUD::Build(Game& game) {
    HUD::Build(game.GetGameConfig().hudScale);
    m_panelW      = Scaled(160.0f);
    m_margin      = Scaled(8.0f);
    m_lineH       = Scaled(15.0f);
    m_descLineH   = Scaled(13.0f);
    m_headerH     = Scaled(20.0f);
    m_sellH       = Scaled(22.0f);
    m_sellGap     = Scaled(6.0f);
    m_anchorGap   = Scaled(20.0f);
    m_fontSm      = ScaledInt(11.0f);
    m_fontDesc    = ScaledInt(10.0f);
    m_fontHeader  = ScaledInt(14.0f);
    Hide(); // shown only while a tower is selected or hovered
}

static std::vector<std::string> WrapText(const std::string& text, float maxWidth, int fontSize) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string word, current;
    while (stream >> word) {
        std::string candidate = current.empty() ? word : current + " " + word;
        if (MeasureText(candidate.c_str(), fontSize) <= static_cast<int>(maxWidth))
            current = candidate;
        else {
            if (!current.empty()) lines.push_back(current);
            current = word;
        }
    }
    if (!current.empty()) lines.push_back(current);
    return lines;
}

void TowerInfoHUD::SetTarget(Game& game, const Tower& tower, Vector2 screenPos, bool showSell) {
    m_target = &tower;
    m_showSell = showSell;

    int moduleRows = 0;
    for (const auto& mod : tower.m_modules) {
        std::string text; Color color;
        mod->Describe(text, color);
        if (!text.empty()) moduleRows++;
    }

    m_descLines = WrapText(tower.m_description, m_panelW - m_margin * 2.0f, m_fontDesc);

    float panelH = m_margin + m_headerH
        + static_cast<float>(m_descLines.size()) * m_descLineH
        + (3 + moduleRows) * m_lineH
        + (m_showSell ? m_sellGap + m_sellH : 0.0f) + m_margin;

    // Anchor above the screen point, then clamp so the panel stays on-screen
    m_panelRect = { screenPos.x - m_panelW / 2.0f, screenPos.y - panelH - m_anchorGap,
                    m_panelW, panelH };
    ClampPanelToScreen(game.GetScreen().GetGameWidth(), game.GetScreen().GetGameHeight());

    m_sellBtn.m_label = TextFormat("Sell: $%d", tower.m_cost / 2);
    m_sellBtn.m_rect = { m_panelRect.x + m_margin, m_panelRect.y + panelH - m_margin - m_sellH,
                         m_panelW - m_margin * 2.0f, m_sellH };

    Show();
}

void TowerInfoHUD::OnProcessInput(Game& game) {
    bool pressed = game.GetInput().IsMousePressed(MOUSE_LEFT_BUTTON);
    ConsumePanelClick(game.GetInput());
    if (!m_showSell) return; // hover preview has no sell button
    Vector2 mousePos = game.GetInput().GetMousePosition();
    m_sellBtn.Update(mousePos, pressed);
    if (m_sellBtn.IsClicked())
        m_sellSignal.Raise();
}

void TowerInfoHUD::OnDraw(Game& /*game*/) {
    if (!m_target) return;
    const Tower& tower = *m_target;

    DrawPanelBackground(220, true);

    float x = m_panelRect.x + m_margin;
    float y = m_panelRect.y + m_margin;

    // Tower name as header
    DrawText(tower.m_name.c_str(), static_cast<int>(x), static_cast<int>(y), m_fontHeader, GOLD);
    y += m_headerH;

    // Description (word-wrapped, computed in SetTarget)
    for (const auto& line : m_descLines) {
        DrawText(line.c_str(), static_cast<int>(x), static_cast<int>(y), m_fontDesc, {180, 180, 180, 255});
        y += m_descLineH;
    }

    // Core stats
    DrawText(TextFormat("Range:   %.0f",   tower.m_stats.radius),       static_cast<int>(x), static_cast<int>(y), m_fontSm, RAYWHITE); y += m_lineH;
    DrawText(TextFormat("Rate:    %.1f/s", tower.m_stats.fireRate),     static_cast<int>(x), static_cast<int>(y), m_fontSm, RAYWHITE); y += m_lineH;
    DrawText(TextFormat("Targets: %d",     tower.m_stats.targetCount),  static_cast<int>(x), static_cast<int>(y), m_fontSm, RAYWHITE); y += m_lineH;

    // Module-derived stats
    for (const auto& mod : tower.m_modules) {
        std::string text; Color color;
        mod->Describe(text, color);
        if (text.empty()) continue;
        DrawText(text.c_str(), static_cast<int>(x), static_cast<int>(y), m_fontSm, color);
        y += m_lineH;
    }

    if (m_showSell) {
        m_sellBtn.Draw();
        m_sellBtn.DrawLabel(m_fontSm, GREEN);
    }
}
