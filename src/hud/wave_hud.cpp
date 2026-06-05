#include <hud/wave_hud.hpp>
#include <game.hpp>
#include <systems/wave_manager.hpp>
#include <world/enemy.hpp>
#include <world/enemy_modules.hpp>
#include <raylib.h>
#include <string>

void WaveHUD::Build(Game& game, const WaveManager& waveManager) {
    HUD::Build(game.GetGameConfig().hudScale);
    m_panelW    = Scaled(190.0f);
    m_margin    = Scaled(8.0f);
    m_lineH     = Scaled(14.0f);
    m_rowGap    = Scaled(4.0f);
    m_headerH   = Scaled(20.0f);
    m_indent    = Scaled(10.0f);
    m_topOffset = Scaled(42.0f);
    m_fontSm     = ScaledInt(11.0f);
    m_fontHeader = ScaledInt(14.0f);
    m_waveManager = &waveManager;
    Hide(); // hidden by default; shown via the Waves button or the WaveInfo hotkey
}

void WaveHUD::OnProcessInput(Game& game) {
    // Swallow clicks that land on the panel so they don't place/deselect towers underneath.
    ConsumePanelClick(game.GetInput());
}

void WaveHUD::OnDraw(Game& game) {
    if (!m_waveManager) return;

    const std::vector<WaveManager::SpawnGroup>& groups = m_waveManager->GetNextWaveDef().groups;

    // Height: a header row, then a two-line block per enemy group (empty wave -> one note line).
    float bodyH = groups.empty()
        ? m_lineH
        : static_cast<float>(groups.size()) * (2.0f * m_lineH + m_rowGap);
    float panelH = m_margin + m_headerH + bodyH + m_margin;

    float screenW = static_cast<float>(game.GetScreen().GetGameWidth());
    m_panelRect = { screenW - m_panelW - m_margin, m_topOffset, m_panelW, panelH };

    DrawPanelBackground(220, true);

    float x = m_panelRect.x + m_margin;
    float y = m_panelRect.y + m_margin;

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

    const EnemyFactory& factory = game.GetEnemyFactory();
    for (const auto& g : groups) {
        // Row 1: count and enemy type.
        DrawText(TextFormat("%dx %s", g.count, g.enemyType.c_str()),
                 static_cast<int>(x), static_cast<int>(y), m_fontSm, RAYWHITE);
        y += m_lineH;

        // Row 2: compact key stats, indented. Unknown types (e.g. an unlisted boss) show no stats.
        if (factory.Has(g.enemyType)) {
            Enemy enemy = factory.Create(g.enemyType);
            const BaseStatsModule* bs = enemy.GetBaseStats();
            std::string stats = TextFormat("HP %g  SPD %g", bs->m_maxHealth, bs->m_liveSpeed);
            if (bs->m_liveArmor > 0.0f)
                stats += TextFormat("  ARM %g", bs->m_liveArmor);
            if (const ShieldModule* shield = enemy.GetShield())
                stats += TextFormat("  SHD %g", shield->GetShield());
            DrawText(stats.c_str(), static_cast<int>(x + m_indent), static_cast<int>(y),
                     m_fontSm, {150, 200, 150, 255});
        }
        y += m_lineH + m_rowGap;
    }
}
