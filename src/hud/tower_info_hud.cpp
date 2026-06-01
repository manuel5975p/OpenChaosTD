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
    m_showSell      = interactive && !game.GetGameData().waveActive;     // selling stays between waves
    m_showTargeting = interactive && tower.m_role == TowerRole::Shooter; // retarget any time
    m_showUpgrade   = interactive && tower.m_role == TowerRole::Shooter
                      && tower.m_upgrades && !tower.m_upgrades->empty();

    int moduleRows = 0;
    for (const auto& mod : tower.m_modules) {
        std::string text; Color color;
        mod->Describe(text, color);
        if (!text.empty()) moduleRows++;
    }

    m_descLines = WrapText(tower.m_description, m_panelW - m_margin * 2.0f, m_fontDesc);

    // Walls have no combat stats to display. Shooters: Damage/Range/Rate/Targets (+ optional Crit/Pierce) + effect modules
    int extraRows = (tower.m_base.critChance > 0.0f ? 1 : 0) + (tower.m_base.armorPierce > 0.0f ? 1 : 0);
    int statRows = (tower.m_role == TowerRole::Shooter) ? 4 + extraRows + moduleRows : 0;
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

    // Lay config buttons bottom-up: Sell at the bottom, then Targeting, then Upgrade on top
    float btnW = m_panelW - m_margin * 2.0f;
    float btnY = m_panelRect.y + panelH - m_margin - m_sellH;
    if (m_showSell) {
        int refund = static_cast<int>(tower.m_cost * game.GetGameData().sellRefundRate);
        m_sellBtn.m_label = TextFormat("Sell: $%d", refund);
        m_sellBtn.m_rect = { m_panelRect.x + m_margin, btnY, btnW, m_sellH };
        btnY -= m_sellGap + m_sellH;
    }
    if (m_showTargeting) {
        m_targetBtn.m_label = TextFormat("Target: %s", TargetingModeName(tower.m_base.targetingMode));
        m_targetBtn.m_rect = { m_panelRect.x + m_margin, btnY, btnW, m_sellH };
        btnY -= m_sellGap + m_sellH;
    }
    m_upgradeReady = false;
    if (m_showUpgrade) {
        int lvl = tower.m_level;
        int count = static_cast<int>(tower.m_upgrades->size());
        if (lvl >= count) {
            m_upgradeBtn.m_label = "Max Level";
        } else {
            int cost = (*tower.m_upgrades)[lvl].cost;
            m_upgradeBtn.m_label = TextFormat("Upgrade $%d", cost);
            m_upgradeReady = game.GetGameData().gold >= cost;
        }
        m_upgradeBtn.m_rect = { m_panelRect.x + m_margin, btnY, btnW, m_sellH };
    }

    Show();
}

void TowerInfoHUD::OnProcessInput(Game& game) {
    bool pressed = game.GetInput().IsMousePressed(MOUSE_LEFT_BUTTON);
    ConsumePanelClick(game.GetInput());
    Vector2 mousePos = game.GetInput().GetMousePosition();

    if (m_showUpgrade && m_upgradeReady) {
        m_upgradeBtn.Update(mousePos, pressed);
        if (m_upgradeBtn.IsClicked())
            m_upgradeSignal.Raise();
    }
    if (m_showTargeting) {
        m_targetBtn.Update(mousePos, pressed);
        if (m_targetBtn.IsClicked())
            m_targetSignal.Raise();
    }
    if (m_showSell) {
        m_sellBtn.Update(mousePos, pressed);
        if (m_sellBtn.IsClicked())
            m_sellSignal.Raise();
    }
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

    // Core stats — skipped for walls, which have no combat function
    if (tower.m_role == TowerRole::Shooter) {
        DrawText(TextFormat("Damage:  %g",     tower.m_stats.damage),                          static_cast<int>(x), static_cast<int>(y), m_fontSm, RAYWHITE); y += m_lineH;
        DrawText(TextFormat("Range:   %.0f",   tower.m_stats.range),                           static_cast<int>(x), static_cast<int>(y), m_fontSm, RAYWHITE); y += m_lineH;
        DrawText(TextFormat("Rate:    %d/min", static_cast<int>(tower.m_stats.shotsPerMinute + 0.5f)), static_cast<int>(x), static_cast<int>(y), m_fontSm, RAYWHITE); y += m_lineH;
        DrawText(TextFormat("Targets: %d",     tower.m_stats.targetCount),                     static_cast<int>(x), static_cast<int>(y), m_fontSm, RAYWHITE); y += m_lineH;

        if (tower.m_stats.critChance > 0.0f) {
            DrawText(TextFormat("Crit:    %.0f%%  x%.1f", tower.m_stats.critChance * 100.0f, tower.m_stats.critMultiplier),
                     static_cast<int>(x), static_cast<int>(y), m_fontSm, YELLOW); y += m_lineH;
        }
        if (tower.m_stats.armorPierce > 0.0f) {
            DrawText(TextFormat("Pierce:  %g", tower.m_stats.armorPierce),
                     static_cast<int>(x), static_cast<int>(y), m_fontSm, GOLD); y += m_lineH;
        }

        for (const auto& mod : tower.m_modules) {
            std::string text; Color color;
            mod->Describe(text, color);
            if (text.empty()) continue;
            DrawText(text.c_str(), static_cast<int>(x), static_cast<int>(y), m_fontSm, color);
            y += m_lineH;
        }
    }

    if (m_showUpgrade) {
        const WidgetStyle& style = m_upgradeReady ? kDefaultStyle : kDisabledStyle;
        m_upgradeBtn.Draw(false, style);
        m_upgradeBtn.DrawLabel(m_fontSm, m_upgradeReady ? Color{160, 240, 120, 255} : DARKGRAY);
    }

    if (m_showTargeting) {
        m_targetBtn.Draw();
        m_targetBtn.DrawLabel(m_fontSm, SKYBLUE);
    }

    if (m_showSell) {
        m_sellBtn.Draw();
        m_sellBtn.DrawLabel(m_fontSm, GREEN);
    }
}
