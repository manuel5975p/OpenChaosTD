#include <hud/tower_info_hud.hpp>
#include <game.hpp>
#include <world/tower_modules.hpp>
#include <raylib.h>
#include <algorithm>

void TowerInfoHUD::SetAnchor(Vector2 screenPos, int screenW, int screenH, const Tower& tower) {
    // Count how many module rows this specific tower needs
    int moduleRows = 0;
    for (const auto& mod : tower.m_modules) {
        if (dynamic_cast<const FlatDamageModule*>(mod.get())) moduleRows++;
        if (dynamic_cast<const SlowModule*>(mod.get())) moduleRows++;
    }

    // Height = top margin + name header + 3 core stats + module stats + gap + sell button + bottom margin
    constexpr float HEADER_H = 20.0f;
    constexpr float SELL_H   = 22.0f;
    constexpr float SELL_GAP = 6.0f;
    m_panelH = MARGIN + HEADER_H + (3 + moduleRows) * LINE_H + SELL_GAP + SELL_H + MARGIN;

    // Float panel above the tower sprite, centered on it horizontally
    float px = screenPos.x - PANEL_W / 2.0f;
    float py = screenPos.y - m_panelH - 20.0f;

    // Clamp so the panel never goes off-screen
    px = std::clamp(px, 0.0f, static_cast<float>(screenW) - PANEL_W);
    py = std::clamp(py, 0.0f, static_cast<float>(screenH) - m_panelH);

    m_panelRect = { px, py, PANEL_W, m_panelH };

    // Sell button anchored to the bottom of the panel; label set here so Draw stays pure
    m_sellBtn.m_label = TextFormat("Sell: $%d", tower.m_cost / 2);
    m_sellBtn.m_rect  = {
        px + MARGIN,
        py + m_panelH - MARGIN - SELL_H,
        PANEL_W - MARGIN * 2.0f,
        SELL_H
    };
}

void TowerInfoHUD::ProcessInput(Game& game) {
    Vector2 mousePos = game.GetInput().GetMousePosition();

    // Prevent clicks from passing through the panel to the world
    if (game.GetInput().IsPressed("Select") && CheckCollisionPointRec(mousePos, m_panelRect))
        game.GetInput().ConsumeMouseInput();

    if (game.GetInput().IsPressed("Select") && m_sellBtn.IsClicked(mousePos, true))
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

    // Sell button — label and refund amount were set in SetAnchor
    m_sellBtn.Draw(mousePos);
    m_sellBtn.DrawLabel(FONT_SM, GREEN);
}
