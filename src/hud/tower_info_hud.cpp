#include <hud/tower_info_hud.hpp>
#include <game.hpp>
#include <world/tower_modules.hpp>
#include <raylib.h>
#include <algorithm>
#include <sstream>

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

void TowerInfoHUD::SetAnchor(Vector2 screenPos, int screenW, int screenH, const Tower& tower, bool showSell) {
    m_showSell = showSell;
    // Count how many module rows this specific tower needs
    int moduleRows = 0;
    for (const auto& mod : tower.m_modules) {
        if (dynamic_cast<const FlatDamageModule*>(mod.get())) moduleRows++;
        if (dynamic_cast<const SlowModule*>(mod.get())) moduleRows++;
    }

    m_descLines = WrapText(tower.m_description, PANEL_W - MARGIN * 2.0f, FONT_DESC);

    constexpr float HEADER_H = 20.0f;
    constexpr float SELL_H = 22.0f;
    constexpr float SELL_GAP = 6.0f;
    float panelH = MARGIN + HEADER_H
        + static_cast<float>(m_descLines.size()) * DESC_LINE_H
        + (3 + moduleRows) * LINE_H
        + (m_showSell ? SELL_GAP + SELL_H : 0.0f) + MARGIN;

    float px = std::clamp(screenPos.x - PANEL_W / 2.0f, 0.0f, static_cast<float>(screenW) - PANEL_W);
    float py = std::clamp(screenPos.y - panelH - 20.0f,  0.0f, static_cast<float>(screenH) - panelH);

    m_panelRect = { px, py, PANEL_W, panelH };

    m_sellBtn.m_label = TextFormat("Sell: $%d", tower.m_cost / 2);
    m_sellBtn.m_rect = { px + MARGIN, py + panelH - MARGIN - SELL_H, PANEL_W - MARGIN * 2.0f, SELL_H };
}

void TowerInfoHUD::ProcessInput(Game& game) {
    if (!game.GetInput().IsPressed("Select")) return;
    Vector2 mousePos = game.GetInput().GetMousePosition();
    if (CheckCollisionPointRec(mousePos, m_panelRect))
        game.GetInput().ConsumeMouseInput();
    if (m_sellBtn.IsClicked(mousePos, true))
        m_sellRequested = true;
}

bool TowerInfoHUD::WasSellRequested() {
    if (!m_sellRequested) return false;
    m_sellRequested = false;
    return true;
}

void TowerInfoHUD::Draw(Game& game, const Tower& tower) {
    Vector2 mousePos = game.GetInput().GetMousePosition();

    // Panel background and border
    DrawRectangleRec(m_panelRect, {20, 20, 20, 220});
    DrawRectangleLinesEx(m_panelRect, 1.0f, {80, 80, 80, 255});

    float x = m_panelRect.x + MARGIN;
    float y = m_panelRect.y + MARGIN;

    // Tower name as header
    DrawText(tower.m_name.c_str(), static_cast<int>(x), static_cast<int>(y), 14, GOLD);
    y += 20.0f;

    // Description (word-wrapped, computed in SetAnchor)
    for (const auto& line : m_descLines) {
        DrawText(line.c_str(), static_cast<int>(x), static_cast<int>(y), FONT_DESC, {180, 180, 180, 255});
        y += DESC_LINE_H;
    }

    // Core stats
    DrawText(TextFormat("Range:   %.0f",   tower.m_radius),     static_cast<int>(x), static_cast<int>(y), FONT_SM, RAYWHITE); y += LINE_H;
    DrawText(TextFormat("Rate:    %.1f/s", tower.m_fireRate),   static_cast<int>(x), static_cast<int>(y), FONT_SM, RAYWHITE); y += LINE_H;
    DrawText(TextFormat("Targets: %d",     tower.m_targetCount), static_cast<int>(x), static_cast<int>(y), FONT_SM, RAYWHITE); y += LINE_H;

    // Module-derived stats — only show what the tower actually has
    for (const auto& mod : tower.m_modules) {
        if (auto* dmg = dynamic_cast<FlatDamageModule*>(mod.get())) {
            DrawText(TextFormat("Damage:  %.0f", dmg->m_damage),
                static_cast<int>(x), static_cast<int>(y), FONT_SM, RAYWHITE);
            y += LINE_H;
        }
        if (auto* slow = dynamic_cast<SlowModule*>(mod.get())) {
            DrawText(TextFormat("Slow:    %.0f%%  %.1fs", (1.0f - slow->m_factor) * 100.0f, slow->m_duration),
                static_cast<int>(x), static_cast<int>(y), FONT_SM, SKYBLUE);
            y += LINE_H;
        }
    }

    if (m_showSell) {
        m_sellBtn.Draw(mousePos);
        m_sellBtn.DrawLabel(FONT_SM, GREEN);
    }
}
