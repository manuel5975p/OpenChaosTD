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

void TowerInfoHUD::SetTarget(Game& game, const Tower& tower, Vector2 screenPos, bool interactive) {
    m_target = &tower;
    const AttackModule* attack = tower.GetAttack(); // null = a wall (no combat UI)
    m_showSell      = interactive;                                    // always shown for a selected tower
    m_sellEnabled   = !game.GetGameData().m_waveActive;              // but only sellable between waves
    m_showTargeting = interactive && attack;                          // retarget any time
    m_showUpgrade   = interactive && attack
                      && tower.m_upgrades && !tower.m_upgrades->empty();

    m_descLines = WrapText(tower.m_description, m_panelW - m_margin * 2.0f, m_fontDesc);

    // Every module appends its rows: AttackModule's core stats, then each effect module's lines
    // (incl. Pierce/Crit). Walls carry only a PassiveModule, which adds nothing.
    std::vector<DescLine> statLines;
    for (const auto& mod : tower.m_modules)
        mod->DescribeStats(statLines);
    int statRows = static_cast<int>(statLines.size());
    float panelH = m_margin + m_headerH
        + static_cast<float>(m_descLines.size()) * m_descLineH
        + statRows * m_lineH
        + (m_showUpgrade ? m_sellGap + m_sellH : 0.0f)
        + (m_showTargeting ? m_sellGap + m_sellH : 0.0f)
        + (m_showSell ? m_sellGap + m_sellH : 0.0f) + m_margin;

    // Anchor above the screen point, then clamp so the panel stays on-screen
    m_panelRect = { screenPos.x - m_panelW / 2.0f, screenPos.y - panelH - m_anchorGap,
                    m_panelW, panelH };
    ClampPanelToScreen(game.GetScreen().GetGameWidth(), game.GetScreen().GetGameHeight());

    // Lay config buttons bottom-up: Sell at the bottom, then Upgrade, then Targeting on top
    float btnW = m_panelW - m_margin * 2.0f;
    float btnY = m_panelRect.y + panelH - m_margin - m_sellH;
    if (m_showSell) {
        int refund = static_cast<int>(tower.m_cost * game.GetGameData().m_sellRefundRate);
        m_sellBtn.m_label = TextFormat("Sell: $%d", refund);
        m_sellBtn.m_rect = { m_panelRect.x + m_margin, btnY, btnW, m_sellH };
        btnY -= m_sellGap + m_sellH;
    }
    m_upgradeReady = false;
    m_hasNextUpgrade = false;
    m_upgradePreview.clear();
    if (m_showUpgrade) {
        int lvl = tower.m_level;
        int count = static_cast<int>(tower.m_upgrades->size());
        if (lvl >= count) {
            m_upgradeBtn.m_label = "Max Level";
        } else {
            const TowerUpgrade& up = (*tower.m_upgrades)[lvl];
            m_upgradeBtn.m_label = TextFormat("Upgrade $%d", up.m_cost);
            m_upgradeReady = game.GetGameData().m_gold >= up.m_cost;
            m_hasNextUpgrade = true;
            up.Describe(m_upgradePreview);
        }
        m_upgradeBtn.m_rect = { m_panelRect.x + m_margin, btnY, btnW, m_sellH };
        btnY -= m_sellGap + m_sellH;
    }
    if (m_showTargeting) {
        m_targetBtn.m_label = TextFormat("Target: %s", TargetingModeName(attack->m_targetingMode));
        m_targetBtn.m_rect = { m_panelRect.x + m_margin, btnY, btnW, m_sellH };
    }

    Show();
}

void TowerInfoHUD::OnProcessInput(Game& game) {
    bool pressed = game.GetInput().IsMousePressed(MOUSE_LEFT_BUTTON);
    ConsumePanelClick(game.GetInput());
    Vector2 mousePos = game.GetInput().GetMousePosition();

    // Update whenever an upgrade is available so hover registers even when unaffordable,
    // but only raise the buy signal when the player can pay.
    if (m_hasNextUpgrade) {
        m_upgradeBtn.Update(mousePos, pressed);
        if (m_upgradeReady && m_upgradeBtn.IsClicked())
            m_upgradeSignal.Raise();
    }
    if (m_showTargeting) {
        m_targetBtn.Update(mousePos, pressed);
        if (m_targetBtn.IsClicked())
            m_targetSignal.Raise();
    }
    // Sell is shown while disabled during waves, but only a wave-free (enabled) button reacts.
    if (m_showSell && m_sellEnabled) {
        m_sellBtn.Update(mousePos, pressed);
        if (m_sellBtn.IsClicked())
            m_sellSignal.Raise();
    }
}

void TowerInfoHUD::OnDraw(Game& game) {
    if (!m_target) return;
    const Tower& tower = *m_target;

    DrawPanelBackground(220, true);

    float x = m_panelRect.x + m_margin;
    float y = m_panelRect.y + m_margin;

    const AttackModule* attack = tower.GetAttack(); // null = a wall (no combat UI)

    // Tower name as header, with level indicator right-aligned
    DrawText(tower.m_name.c_str(), static_cast<int>(x), static_cast<int>(y), m_fontHeader, GOLD);
    if (attack && tower.m_upgrades && !tower.m_upgrades->empty()) {
        int maxLvl = static_cast<int>(tower.m_upgrades->size());
        bool isMax = tower.m_level >= maxLvl;
        const char* lvlText = isMax ? "MAX" : TextFormat("Lv %d", tower.m_level + 1);
        Color lvlColor = isMax ? GOLD : Color{180, 180, 180, 255};
        int tw = MeasureText(lvlText, m_fontHeader);
        DrawText(lvlText,
                 static_cast<int>(m_panelRect.x + m_panelW - m_margin) - tw,
                 static_cast<int>(y), m_fontHeader, lvlColor);
    }
    y += m_headerH;

    // Description (word-wrapped, computed in SetTarget)
    for (const auto& line : m_descLines) {
        DrawText(line.c_str(), static_cast<int>(x), static_cast<int>(y), m_fontDesc, {180, 180, 180, 255});
        y += m_descLineH;
    }

    // Stat rows from every module (AttackModule core stats + effect lines). Walls add nothing.
    std::vector<DescLine> statLines;
    for (const auto& mod : tower.m_modules)
        mod->DescribeStats(statLines);
    for (const auto& line : statLines) {
        DrawText(line.m_text.c_str(), static_cast<int>(x), static_cast<int>(y), m_fontSm, line.m_color);
        y += m_lineH;
    }

    if (m_showUpgrade) {
        const WidgetStyle& style = m_upgradeReady ? kDefaultStyle : kDisabledStyle;
        m_upgradeBtn.Draw(false, style);
        m_upgradeBtn.DrawLabel(m_fontSm, m_upgradeReady ? Color{160, 240, 120, 255} : DARKGRAY);
        if (m_hasNextUpgrade && m_upgradeBtn.IsHovered())
            DrawUpgradeTooltip(game);
    }

    if (m_showTargeting) {
        m_targetBtn.Draw();
        m_targetBtn.DrawLabel(m_fontSm, SKYBLUE);
    }

    if (m_showSell) {
        const WidgetStyle& style = m_sellEnabled ? kDefaultStyle : kDisabledStyle;
        m_sellBtn.Draw(false, style);
        m_sellBtn.DrawLabel(m_fontSm, m_sellEnabled ? GREEN : DARKGRAY);
    }
}

void TowerInfoHUD::DrawUpgradeTooltip(Game& game) {
    // Box sized to the widest of the header and the preview lines
    int maxW = MeasureText("Next Level", m_fontSm);
    for (const auto& line : m_upgradePreview)
        maxW = std::max(maxW, MeasureText(line.m_text.c_str(), m_fontSm));

    float boxW = maxW + m_margin * 2.0f;
    float boxH = m_margin * 2.0f + static_cast<float>(m_upgradePreview.size() + 1) * m_lineH;

    // Prefer the left of the panel; flip to the right if it would clip off-screen
    float boxX = m_panelRect.x - boxW - m_sellGap;
    if (boxX < 0.0f)
        boxX = m_panelRect.x + m_panelW + m_sellGap;

    // Align with the upgrade button, clamped to stay on-screen vertically
    float boxY = m_upgradeBtn.m_rect.y;
    float screenH = static_cast<float>(game.GetScreen().GetGameHeight());
    if (boxY + boxH > screenH) boxY = screenH - boxH;
    if (boxY < 0.0f) boxY = 0.0f;

    Rectangle box = { boxX, boxY, boxW, boxH };
    DrawRectangleRec(box, {20, 20, 20, 235});
    DrawRectangleLinesEx(box, 1.0f, {255, 180, 0, 255});

    float tx = boxX + m_margin;
    float ty = boxY + m_margin;
    DrawText("Next Level", static_cast<int>(tx), static_cast<int>(ty), m_fontSm, GOLD);
    ty += m_lineH;
    for (const auto& line : m_upgradePreview) {
        DrawText(line.m_text.c_str(), static_cast<int>(tx), static_cast<int>(ty), m_fontSm, line.m_color);
        ty += m_lineH;
    }
}
