#include <hud/tower_info_hud.hpp>
#include <engine/core/text.hpp>
#include <engine/core/input.hpp>
#include <raylib.h>
#include <sstream>
#include <algorithm>

void TowerInfoHUD::Build(float scale) {
    HUD::Build(scale);
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
        if (Text::Measure(candidate.c_str(), fontSize) <= static_cast<int>(maxWidth))
            current = candidate;
        else {
            if (!current.empty()) lines.push_back(current);
            current = word;
        }
    }
    if (!current.empty()) lines.push_back(current);
    return lines;
}

void TowerInfoHUD::SetTarget(const TowerInfoView& view) {
    // Snapshot the content the panel will render.
    m_hasTarget     = true;
    m_name          = view.m_name;
    m_hasAttack     = view.m_hasAttack;
    m_level         = view.m_level;
    m_upgradeCount  = view.m_upgradeCount;
    m_statLines     = view.m_statLines;
    m_targetingName = view.m_targetingName;
    m_screenH       = view.m_screenH;

    m_showSell      = view.m_interactive;                              // always shown for a selected tower
    m_sellEnabled   = !view.m_waveActive;                             // but only sellable between waves
    m_showTargeting = view.m_interactive && view.m_hasAttack;         // retarget any time
    m_showUpgrade   = view.m_interactive && view.m_hasAttack && view.m_upgradeCount > 0;

    m_descLines = WrapText(view.m_description, m_panelW - m_margin * 2.0f, m_fontDesc);

    int statRows = static_cast<int>(m_statLines.size());
    float panelH = m_margin + m_headerH
        + static_cast<float>(m_descLines.size()) * m_descLineH
        + statRows * m_lineH
        + (m_showUpgrade ? m_sellGap + m_sellH : 0.0f)
        + (m_showTargeting ? m_sellGap + m_sellH : 0.0f)
        + (m_showSell ? m_sellGap + m_sellH : 0.0f) + m_margin;

    // Anchor above the screen point, then clamp so the panel stays on-screen
    m_panelRect = { view.m_screenPos.x - m_panelW / 2.0f, view.m_screenPos.y - panelH - m_anchorGap,
                    m_panelW, panelH };
    ClampPanelToScreen(view.m_screenW, view.m_screenH);

    // Lay config buttons bottom-up: Sell at the bottom, then Upgrade, then Targeting on top
    float btnW = m_panelW - m_margin * 2.0f;
    float btnY = m_panelRect.y + panelH - m_margin - m_sellH;
    if (m_showSell) {
        m_sellBtn.m_label = TextFormat("Sell: $%d", view.m_sellRefund);
        m_sellBtn.m_rect = { m_panelRect.x + m_margin, btnY, btnW, m_sellH };
        btnY -= m_sellGap + m_sellH;
    }
    m_upgradeReady = false;
    m_hasNextUpgrade = false;
    m_upgradePreview.clear();
    if (m_showUpgrade) {
        if (view.m_upgradeAtMax) {
            m_upgradeBtn.m_label = "Max Level";
        } else {
            m_upgradeBtn.m_label = TextFormat("Upgrade $%d", view.m_upgradeCost);
            m_upgradeReady = view.m_upgradeReady;
            m_hasNextUpgrade = true;
            m_upgradePreview = view.m_upgradePreview;
        }
        m_upgradeBtn.m_rect = { m_panelRect.x + m_margin, btnY, btnW, m_sellH };
        btnY -= m_sellGap + m_sellH;
    }
    if (m_showTargeting) {
        m_targetBtn.m_label = TextFormat("Target: %s", m_targetingName.c_str());
        m_targetBtn.m_rect = { m_panelRect.x + m_margin, btnY, btnW, m_sellH };
    }

    Show();
}

void TowerInfoHUD::ProcessInput(Input& input) {
    if (!m_visible) return;

    bool pressed = input.IsMousePressed(MOUSE_LEFT_BUTTON);
    ConsumePanelClick(input);
    Vector2 mousePos = input.GetMousePosition();

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

void TowerInfoHUD::Draw() {
    if (!m_visible || !m_hasTarget) return;

    DrawPanelBackground(220, true);

    float x = m_panelRect.x + m_margin;
    float y = m_panelRect.y + m_margin;

    // Tower name as header, with level indicator right-aligned
    Text::Draw(m_name.c_str(), static_cast<int>(x), static_cast<int>(y), m_fontHeader, GOLD);
    if (m_hasAttack && m_upgradeCount > 0) {
        bool isMax = m_level >= m_upgradeCount;
        const char* lvlText = isMax ? "MAX" : TextFormat("Lv %d", m_level + 1);
        Color lvlColor = isMax ? GOLD : Color{180, 180, 180, 255};
        int tw = Text::Measure(lvlText, m_fontHeader);
        Text::Draw(lvlText,
                 static_cast<int>(m_panelRect.x + m_panelW - m_margin) - tw,
                 static_cast<int>(y), m_fontHeader, lvlColor);
    }
    y += m_headerH;

    // Description (word-wrapped, computed in SetTarget)
    for (const auto& line : m_descLines) {
        Text::Draw(line.c_str(), static_cast<int>(x), static_cast<int>(y), m_fontDesc, {180, 180, 180, 255});
        y += m_descLineH;
    }

    // Stat rows from every module (AttackModule core stats + effect lines). Walls add nothing.
    for (const auto& line : m_statLines) {
        Text::Draw(line.m_text.c_str(), static_cast<int>(x), static_cast<int>(y), m_fontSm, line.m_color);
        y += m_lineH;
    }

    if (m_showUpgrade) {
        const WidgetStyle& style = m_upgradeReady ? kDefaultStyle : kDisabledStyle;
        m_upgradeBtn.Draw(false, style);
        m_upgradeBtn.DrawLabel(m_fontSm, m_upgradeReady ? Color{160, 240, 120, 255} : DARKGRAY);
        if (m_hasNextUpgrade && m_upgradeBtn.IsHovered())
            DrawUpgradeTooltip();
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

void TowerInfoHUD::DrawUpgradeTooltip() {
    // Box sized to the widest of the header and the preview lines
    int maxW = Text::Measure("Next Level", m_fontSm);
    for (const auto& line : m_upgradePreview)
        maxW = std::max(maxW, Text::Measure(line.m_text.c_str(), m_fontSm));

    float boxW = maxW + m_margin * 2.0f;
    float boxH = m_margin * 2.0f + static_cast<float>(m_upgradePreview.size() + 1) * m_lineH;

    // Prefer the left of the panel; flip to the right if it would clip off-screen
    float boxX = m_panelRect.x - boxW - m_sellGap;
    if (boxX < 0.0f)
        boxX = m_panelRect.x + m_panelW + m_sellGap;

    // Align with the upgrade button, clamped to stay on-screen vertically
    float boxY = m_upgradeBtn.m_rect.y;
    float screenH = static_cast<float>(m_screenH);
    if (boxY + boxH > screenH) boxY = screenH - boxH;
    if (boxY < 0.0f) boxY = 0.0f;

    Rectangle box = { boxX, boxY, boxW, boxH };
    DrawRectangleRec(box, {20, 20, 20, 235});
    DrawRectangleLinesEx(box, 1.0f, {255, 180, 0, 255});

    float tx = boxX + m_margin;
    float ty = boxY + m_margin;
    Text::Draw("Next Level", static_cast<int>(tx), static_cast<int>(ty), m_fontSm, GOLD);
    ty += m_lineH;
    for (const auto& line : m_upgradePreview) {
        Text::Draw(line.m_text.c_str(), static_cast<int>(tx), static_cast<int>(ty), m_fontSm, line.m_color);
        ty += m_lineH;
    }
}
