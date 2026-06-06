#include <hud/wave_hud.hpp>
#include <hud/wave_enemy_card.hpp>
#include <game.hpp>
#include <systems/wave_manager.hpp>
#include <systems/render_system.hpp>
#include <world/enemy.hpp>
#include <raylib.h>
#include <vector>
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

    // Build one card per enemy group up front, so the panel can size itself to their total height
    // (cards grow with their module count, exactly like the tower info panel).
    std::vector<WaveEnemyCard> cards;
    cards.reserve(groups.size());
    for (const auto& g : groups) {
        WaveEnemyCard card;
        card.m_pad = m_cardPad;
        card.m_iconSize = m_iconSize;
        card.m_lineH = m_lineH;
        card.m_fontSm = m_fontSm;
        auto it = prototypes.find(g.enemyType);
        card.SetContent(g.count, g.enemyType, it != prototypes.end() ? &it->second : nullptr);
        cards.push_back(std::move(card));
    }

    // An empty wave collapses to a single note line; otherwise sum the cards plus the gaps.
    float bodyH = m_lineH;
    if (!cards.empty()) {
        bodyH = static_cast<float>(cards.size() - 1) * m_cardGap;
        for (const auto& card : cards)
            bodyH += card.Measure();
    }
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

    if (cards.empty()) {
        DrawText("No enemies", static_cast<int>(x), static_cast<int>(y), m_fontSm, {180, 180, 180, 255});
        return;
    }

    for (const auto& card : cards) {
        float h = card.Measure();
        card.Draw({ x, y, innerW, h }, *m_renderSystem, game.GetResources());
        y += h + m_cardGap;
    }
}
