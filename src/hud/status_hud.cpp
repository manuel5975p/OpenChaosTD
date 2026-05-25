#include <hud/status_hud.hpp>
#include <game.hpp>
#include <raylib.h>

void StatusHUD::Build(Game& game) {
    HUD::Build(game.GetGameConfig().hudScale);
    float panelH   = Scaled(36.0f);
    float btnH     = Scaled(24.0f);
    float btnWaveW = Scaled(90.0f);
    float btnAutoW = Scaled(48.0f);
    float margin   = Scaled(6.0f);
    float w = static_cast<float>(game.GetScreen().GetGameWidth());

    m_panelRect = { 0.0f, 0.0f, w, panelH };
    m_textY = static_cast<int>((panelH - Scaled(16.0f)) / 2.0f);

    float btnY = (panelH - btnH) / 2.0f;
    m_startWaveBtn.m_label = "Start Wave";
    m_startWaveBtn.m_rect = { w - btnWaveW - margin, btnY, btnWaveW, btnH };
    m_autoBtn.m_label = "Auto";
    m_autoBtn.m_rect = { w - btnWaveW - margin - btnAutoW - margin, btnY, btnAutoW, btnH };
}

void StatusHUD::OnProcessInput(Game& game) {
    const auto& data = game.GetGameData();
    Vector2 mousePos = game.GetInput().GetMousePosition();

    ConsumePanelClick(game.GetInput());

    if (game.GetInput().IsMousePressed(MOUSE_LEFT_BUTTON)) {
        // Auto toggle is always clickable, even mid-wave
        if (m_autoBtn.IsClicked(mousePos, true))
            m_autoSignal.Raise();

        // Start wave only when no wave is running
        if (!data.waveActive && m_startWaveBtn.IsClicked(mousePos, true))
            m_waveSignal.Raise();
    }
}

void StatusHUD::OnDraw(Game& game) {
    const auto& data     = game.GetGameData();
    Vector2     mousePos = game.GetInput().GetMousePosition();

    DrawPanelBackground(200);

    int fontMain = ScaledInt(16.0f);
    int fontBtn  = ScaledInt(12.0f);
    int marginX  = ScaledInt(6.0f);
    int goldX    = marginX + ScaledInt(110.0f);

    // Left side: lives and gold
    DrawText(TextFormat("Lives: %d", data.lives), marginX, m_textY, fontMain, RAYWHITE);
    DrawText(TextFormat("Gold: %d",  data.gold),  goldX,   m_textY, fontMain, GOLD);

    // Center: wave number and elapsed time
    const char* waveStr;
    if (data.waveNumber == 0)
        waveStr = "Wave: --";
    else if (data.waveActive)
        waveStr = TextFormat("Wave: %d  |  %.1fs", data.waveNumber, data.waveTimer);
    else
        waveStr = TextFormat("Wave: %d  |  Done", data.waveNumber);

    DrawTextCenteredX(waveStr, static_cast<int>(m_panelRect.width / 2.0f), m_textY, fontMain, RAYWHITE);

    // Auto toggle — highlighted when active
    m_autoBtn.Draw(mousePos, m_autoSpawn);
    m_autoBtn.DrawLabel(fontBtn, m_autoSpawn ? GOLD : RAYWHITE);

    // Start wave button — greyed out while a wave is running
    if (data.waveActive) {
        DrawRectangleRec(m_startWaveBtn.m_rect, {30, 30, 30, 200});
        DrawRectangleLinesEx(m_startWaveBtn.m_rect, 1.0f, {60, 60, 60, 255});
        m_startWaveBtn.DrawLabel(fontBtn, DARKGRAY);
    } else {
        m_startWaveBtn.Draw(mousePos);
        m_startWaveBtn.DrawLabel(fontBtn, RAYWHITE);
    }
}
