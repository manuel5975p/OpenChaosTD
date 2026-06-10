#include <hud/wave_hud.hpp>
#include <engine/core/text.hpp>
#include <hud/wave_enemy_card.hpp>
#include <engine/core/input.hpp>
#include <engine/core/resources.hpp>
#include <raylib.h>
#include <vector>

void WaveHUD::Build(float scale, int screenW) {
    HUD::Build(scale);
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
    m_screenW = screenW;
    Hide(); // hidden by default; shown via the Waves button or the WaveInfo hotkey
}

void WaveHUD::ProcessInput(Input& input) {
    if (!m_visible) return;
    // Swallow clicks that land on the panel so they don't place/deselect towers underneath.
    ConsumePanelClick(input);
}

void WaveHUD::Draw(const WaveView& view, Resources& assets) {
    if (!m_visible) return;

    // Build one card per enemy entry up front, so the panel can size itself to their total height
    // (cards grow with their module count, exactly like the tower info panel).
    std::vector<WaveEnemyCard> cards;
    cards.reserve(view.m_entries.size());
    for (const auto& entry : view.m_entries) {
        WaveEnemyCard card;
        card.m_pad = m_cardPad;
        card.m_iconSize = m_iconSize;
        card.m_lineH = m_lineH;
        card.m_fontSm = m_fontSm;
        card.SetContent(entry);
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

    float screenW = static_cast<float>(m_screenW);
    m_panelRect = { screenW - m_panelW - m_margin, m_topOffset, m_panelW, panelH };

    DrawPanelBackground(220, true);

    float x = m_panelRect.x + m_margin;
    float y = m_panelRect.y + m_margin;
    float innerW = m_panelW - 2.0f * m_margin;

    // Header: title on the left, upcoming threat budget right-aligned.
    Text::Draw("Next Wave", static_cast<int>(x), static_cast<int>(y), m_fontHeader, GOLD);
    const char* budgetText = TextFormat("Budget: %d", static_cast<int>(view.m_budget));
    int bw = Text::Measure(budgetText, m_fontSm);
    Text::Draw(budgetText,
             static_cast<int>(m_panelRect.x + m_panelW - m_margin) - bw,
             static_cast<int>(y + (m_fontHeader - m_fontSm)), m_fontSm, {180, 180, 180, 255});
    y += m_headerH;

    if (cards.empty()) {
        Text::Draw("No enemies", static_cast<int>(x), static_cast<int>(y), m_fontSm, {180, 180, 180, 255});
        return;
    }

    for (const auto& card : cards) {
        float h = card.Measure();
        card.Draw({ x, y, innerW, h }, assets);
        y += h + m_cardGap;
    }
}
