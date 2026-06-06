#include <hud/wave_hud.hpp>
#include <game.hpp>
#include <systems/wave_manager.hpp>
#include <systems/render_system.hpp>
#include <world/enemy.hpp>
#include <world/enemy_modules.hpp>
#include <raylib.h>
#include <string>
#include <unordered_map>

void WaveHUD::Build(Game& game, const WaveManager& waveManager, const RenderSystem& renderSystem) {
    HUD::Build(game.GetGameConfig().hudScale);
    m_panelW    = Scaled(200.0f);
    m_margin    = Scaled(8.0f);
    m_lineH     = Scaled(14.0f);
    m_headerH   = Scaled(20.0f);
    m_cardGap   = Scaled(6.0f);
    m_cardPad   = Scaled(6.0f);
    m_iconSize  = Scaled(44.0f);
    m_topOffset = Scaled(42.0f);
    m_fontSm     = ScaledInt(11.0f);
    m_fontHeader = ScaledInt(14.0f);
    m_waveManager = &waveManager;
    m_renderSystem = &renderSystem;
    Hide(); // hidden by default; shown via the Waves button or the WaveInfo hotkey
}

void WaveHUD::OnProcessInput(Game& game) {
    // Swallow clicks that land on the panel so they don't place/deselect towers underneath.
    ConsumePanelClick(game.GetInput());
}

void WaveHUD::OnDraw(Game& game) {
    if (!m_waveManager || !m_renderSystem) return;

    const std::vector<WaveManager::SpawnGroup>& groups = m_waveManager->GetNextWaveDef().groups;
    const std::unordered_map<std::string, Enemy>& prototypes = m_waveManager->GetPreviewPrototypes();

    // One card per enemy group; an empty wave collapses to a single note line.
    float cardH = m_iconSize + 2.0f * m_cardPad;
    float bodyH = groups.empty()
        ? m_lineH
        : static_cast<float>(groups.size()) * (cardH + m_cardGap) - m_cardGap;
    float panelH = m_margin + m_headerH + bodyH + m_margin;

    float screenW = static_cast<float>(game.GetScreen().GetGameWidth());
    m_panelRect = { screenW - m_panelW - m_margin, m_topOffset, m_panelW, panelH };

    DrawPanelBackground(220, true);

    float x = m_panelRect.x + m_margin;
    float y = m_panelRect.y + m_margin;
    float innerW = m_panelW - 2.0f * m_margin;

    // Header: title on the left, upcoming threat budget right-aligned.
    DrawText("Next Wave", static_cast<int>(x), static_cast<int>(y), m_fontHeader, GOLD);
    const char* budgetText = TextFormat("Budget: %d", static_cast<int>(m_waveManager->GetNextWaveBudget()));
    int bw = MeasureText(budgetText, m_fontSm);
    DrawText(budgetText,
             static_cast<int>(m_panelRect.x + m_panelW - m_margin) - bw,
             static_cast<int>(y + (m_fontHeader - m_fontSm)), m_fontSm, {180, 180, 180, 255});
    y += m_headerH;

    if (groups.empty()) {
        DrawText("No enemies", static_cast<int>(x), static_cast<int>(y), m_fontSm, {180, 180, 180, 255});
        return;
    }

    for (const auto& g : groups) {
        // Card frame: subtle fill plus a border so each enemy type reads as a distinct tile.
        Rectangle card = { x, y, innerW, cardH };
        DrawRectangleRec(card, {40, 40, 48, 200});
        DrawRectangleLinesEx(card, 1.0f, {90, 90, 100, 255});

        // Icon area on the left, with a darker backing for sprite contrast.
        Rectangle icon = { card.x + m_cardPad, card.y + m_cardPad, m_iconSize, m_iconSize };
        DrawRectangleRec(icon, {20, 20, 24, 255});

        auto it = prototypes.find(g.enemyType);
        const Enemy* proto = (it != prototypes.end()) ? &it->second : nullptr;
        if (proto)
            m_renderSystem->DrawEnemyIcon(proto->m_visual.m_texture, game.GetResources(), icon);

        // Text column to the right of the icon.
        float tx = icon.x + m_iconSize + m_cardPad;
        float ty = card.y + m_cardPad;
        float textRight = card.x + card.width - m_cardPad;

        // Title row: "Nx Name" on the left, "Lv N" badge right-aligned.
        DrawText(TextFormat("%dx %s", g.count, g.enemyType.c_str()),
                 static_cast<int>(tx), static_cast<int>(ty), m_fontSm, RAYWHITE);
        if (proto) {
            const char* lvlText = TextFormat("Lv %d", proto->m_level);
            int lw = MeasureText(lvlText, m_fontSm);
            DrawText(lvlText, static_cast<int>(textRight) - lw, static_cast<int>(ty), m_fontSm, GOLD);
        }
        ty += m_lineH;

        // Stat rows read the upgraded prototype; unknown types (e.g. an unlisted boss) stay blank.
        if (proto) {
            const BaseStatsModule* bs = proto->GetBaseStats();
            DrawText(TextFormat("HP %g  SPD %g", bs->m_maxHealth, bs->m_liveSpeed),
                     static_cast<int>(tx), static_cast<int>(ty), m_fontSm, {150, 200, 150, 255});
            ty += m_lineH;

            std::string row2;
            if (bs->m_liveArmor > 0.0f)
                row2 += TextFormat("ARM %g", bs->m_liveArmor);
            if (const ShieldModule* shield = proto->GetShield()) {
                if (!row2.empty()) row2 += "  ";
                row2 += TextFormat("SHD %g", shield->GetShield());
            }
            if (!row2.empty())
                DrawText(row2.c_str(), static_cast<int>(tx), static_cast<int>(ty),
                         m_fontSm, {150, 200, 150, 255});
        }

        y += cardH + m_cardGap;
    }
}
