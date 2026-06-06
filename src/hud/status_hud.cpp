#include <hud/status_hud.hpp>
#include <game.hpp>
#include <systems/wave_manager.hpp>
#include <raylib.h>
#include <cstdio>

void StatusHUD::Build(Game& game, const WaveManager& waveManager) {
    HUD::Build(game.GetGameConfig().hudScale);
    m_waveManager = &waveManager;
    float panelH    = Scaled(36.0f);
    float btnH      = Scaled(24.0f);
    float btnWaveW  = Scaled(90.0f);
    float btnAutoW  = Scaled(48.0f);
    float btnWavesW = Scaled(56.0f);
    float margin    = Scaled(6.0f);
    float w = static_cast<float>(game.GetScreen().GetGameWidth());

    m_panelRect = { 0.0f, 0.0f, w, panelH };
    m_textY = static_cast<int>((panelH - Scaled(16.0f)) / 2.0f);

    float btnY = (panelH - btnH) / 2.0f;
    m_startWaveBtn.m_label = "Start Wave";
    m_startWaveBtn.m_rect = { w - btnWaveW - margin, btnY, btnWaveW, btnH };
    m_autoBtn.m_label = "Auto";
    m_autoBtn.m_rect = { w - btnWaveW - margin - btnAutoW - margin, btnY, btnAutoW, btnH };
    m_speedBtn.m_label = "1x";
    m_speedBtn.m_rect = { w - btnWaveW - margin - (btnAutoW + margin) * 2.0f, btnY, btnAutoW, btnH };
    m_waveInfoBtn.m_label = "Waves";
    m_waveInfoBtn.m_rect = { w - btnWaveW - margin - (btnAutoW + margin) * 2.0f - btnWavesW - margin,
                             btnY, btnWavesW, btnH };
}

void StatusHUD::OnProcessInput(Game& game) {
    const auto& data = game.GetGameData();
    Vector2 mousePos = game.GetInput().GetMousePosition();
    bool pressed = game.GetInput().IsMousePressed(MOUSE_LEFT_BUTTON);

    ConsumePanelClick(game.GetInput());

    m_autoBtn.Update(mousePos, pressed);
    m_startWaveBtn.Update(mousePos, pressed);
    m_speedBtn.Update(mousePos, pressed);
    m_waveInfoBtn.Update(mousePos, pressed);

    // Auto toggle is always clickable, even mid-wave
    if (m_autoBtn.IsClicked())
        m_autoSignal.Raise();

    // Speed cycle is always clickable, even mid-wave
    if (m_speedBtn.IsClicked())
        m_speedSignal.Raise();

    // Wave info panel toggle is always clickable
    if (m_waveInfoBtn.IsClicked())
        m_waveInfoSignal.Raise();

    // Start wave only when no wave is running
    if (!data.m_waveActive && m_startWaveBtn.IsClicked())
        m_waveSignal.Raise();
}

void StatusHUD::OnDraw(Game& game) {
    const auto& data = game.GetGameData();

    DrawPanelBackground(200);

    int fontMain  = ScaledInt(16.0f);
    int fontBtn   = ScaledInt(12.0f);
    int marginX   = ScaledInt(6.0f);
    int gapX      = ScaledInt(16.0f);

    // Left cluster: lives, then gold placed after the measured lives width so a large value never
    // runs under the centered readout.
    const char* livesStr = TextFormat("Lives: %d", data.m_lives);
    DrawText(livesStr, marginX, m_textY, fontMain, RAYWHITE);
    int goldX = marginX + MeasureText(livesStr, fontMain) + gapX;
    DrawText(TextFormat("Gold: %d", data.m_gold), goldX, m_textY, fontMain, GOLD);

    // Center: wave progress, win target, and timer folded into one readout (endless => infinity).
    DrawWaveReadout(game, static_cast<int>(m_panelRect.width / 2.0f));

    // Wave info panel toggle
    m_waveInfoBtn.Draw();
    m_waveInfoBtn.DrawLabel(fontBtn, RAYWHITE);

    // Speed cycle — highlighted when faster than 1x
    m_speedBtn.Draw(m_speed > 1);
    m_speedBtn.DrawLabel(fontBtn, m_speed > 1 ? GOLD : RAYWHITE);

    // Auto toggle — highlighted when active
    m_autoBtn.Draw(m_autoSpawn);
    m_autoBtn.DrawLabel(fontBtn, m_autoSpawn ? GOLD : RAYWHITE);

    // Start wave button — greyed out while a wave is running
    const WidgetStyle& waveStyle = data.m_waveActive ? kDisabledStyle : kDefaultStyle;
    m_startWaveBtn.Draw(false, waveStyle);
    m_startWaveBtn.DrawLabel(fontBtn, data.m_waveActive ? DARKGRAY : RAYWHITE);
}

void StatusHUD::DrawWaveReadout(Game& game, int centerX) {
    const auto& data = game.GetGameData();
    int font = ScaledInt(16.0f);

    // Current wave number ("--" before the first wave) and the timer/progress suffix.
    char num[16];
    if (data.m_waveNumber == 0)
        snprintf(num, sizeof(num), "--");
    else
        snprintf(num, sizeof(num), "%d", data.m_waveNumber);

    char suffix[24] = "";
    if (data.m_waveNumber != 0)
        snprintf(suffix, sizeof(suffix), "  |  %s",
                 data.m_waveActive ? TextFormat("%.1fs", data.m_waveTimer) : "Done");

    int victoryWave = m_waveManager ? m_waveManager->GetVictoryWave() : 0;

    // Numeric win target: one plain centered string.
    if (victoryWave > 0) {
        DrawTextCenteredX(TextFormat("Wave: %s / %d%s", num, victoryWave, suffix),
                          centerX, m_textY, font, RAYWHITE);
        return;
    }

    // Endless: the target is the infinity glyph. The default font has no U+221E, so it is drawn by
    // hand; the readout is laid out in measured segments so the whole thing stays centered.
    const char* left = TextFormat("Wave: %s / ", num);
    float glyphH = static_cast<float>(font);
    float glyphW = glyphH * 1.2f;
    float gap = Scaled(2.0f);

    int leftW = MeasureText(left, font);
    int rightW = MeasureText(suffix, font);
    float totalW = static_cast<float>(leftW) + gap + glyphW + gap + static_cast<float>(rightW);
    float startX = static_cast<float>(centerX) - totalW / 2.0f;

    DrawText(left, static_cast<int>(startX), m_textY, font, RAYWHITE);
    float glyphX = startX + static_cast<float>(leftW) + gap;
    DrawInfinity(glyphX, static_cast<float>(m_textY) + glyphH / 2.0f, glyphH, Color{120, 180, 220, 255});
    DrawText(suffix, static_cast<int>(glyphX + glyphW + gap), m_textY, font, RAYWHITE);
}

void StatusHUD::DrawInfinity(float x, float yMid, float h, Color color) const {
    // Two ring outlines side by side form the lemniscate; spans [x, x + 4r] = [x, x + 1.2*h].
    float r = h * 0.30f;
    float th = Scaled(1.5f);
    float inner = r - th;
    if (inner < 0.0f) inner = 0.0f;
    DrawRing({ x + r, yMid },        inner, r, 0.0f, 360.0f, 32, color);
    DrawRing({ x + 3.0f * r, yMid }, inner, r, 0.0f, 360.0f, 32, color);
}
