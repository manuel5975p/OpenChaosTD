#include <hud/status_hud.hpp>
#include <engine/core/text.hpp>
#include <engine/core/input.hpp>
#include <raylib.h>
#include <cstdio>

void StatusHUD::Build(float scale, int screenW) {
    HUD::Build(scale);
    float panelH    = Scaled(36.0f);
    float btnH      = Scaled(24.0f);
    float btnWaveW  = Scaled(90.0f);
    float btnAutoW  = Scaled(48.0f);
    float btnWavesW = Scaled(56.0f);
    float margin    = Scaled(6.0f);
    float w = static_cast<float>(screenW);

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

void StatusHUD::ProcessInput(Input& input, const StatusView& view) {
    if (!m_visible) return;

    Vector2 mousePos = input.GetMousePosition();
    bool pressed = input.IsMousePressed(MOUSE_LEFT_BUTTON);

    ConsumePanelClick(input);

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
    if (!view.m_waveActive && m_startWaveBtn.IsClicked())
        m_waveSignal.Raise();
}

void StatusHUD::Draw(const StatusView& view) {
    if (!m_visible) return;

    DrawPanelBackground(200);

    int fontMain  = ScaledInt(16.0f);
    int fontBtn   = ScaledInt(12.0f);
    int marginX   = ScaledInt(6.0f);
    int gapX      = ScaledInt(16.0f);

    // Left cluster: lives, then gold placed after the measured lives width so a large value never
    // runs under the centered readout.
    const char* livesStr = TextFormat("Lives: %d", view.m_lives);
    Text::Draw(livesStr, marginX, m_textY, fontMain, RAYWHITE, Text::Face::Mono);
    int goldX = marginX + Text::Measure(livesStr, fontMain, Text::Face::Mono) + gapX;
    Text::Draw(TextFormat("Gold: %d", view.m_gold), goldX, m_textY, fontMain, GOLD, Text::Face::Mono);

    // Center: wave progress and win target folded into one readout (endless => infinity).
    DrawWaveReadout(view, static_cast<int>(m_panelRect.width / 2.0f));

    // Wave info panel toggle
    m_waveInfoBtn.Draw();
    m_waveInfoBtn.DrawLabel(fontBtn, RAYWHITE);

    // Speed cycle — highlighted when faster than 1x
    m_speedBtn.m_label = TextFormat("%dx", view.m_speed);
    m_speedBtn.Draw(view.m_speed > 1);
    m_speedBtn.DrawLabel(fontBtn, view.m_speed > 1 ? GOLD : RAYWHITE);

    // Auto toggle — highlighted when active
    m_autoBtn.Draw(view.m_autoSpawn);
    m_autoBtn.DrawLabel(fontBtn, view.m_autoSpawn ? GOLD : RAYWHITE);

    // Start wave button — greyed out while a wave is running
    const WidgetStyle& waveStyle = view.m_waveActive ? kDisabledStyle : kDefaultStyle;
    m_startWaveBtn.Draw(false, waveStyle);
    m_startWaveBtn.DrawLabel(fontBtn, view.m_waveActive ? DARKGRAY : RAYWHITE);
}

void StatusHUD::DrawWaveReadout(const StatusView& view, int centerX) {
    int font = ScaledInt(16.0f);

    // Current wave number ("--" before the first wave).
    char num[16];
    if (view.m_waveNumber == 0)
        snprintf(num, sizeof(num), "--");
    else
        snprintf(num, sizeof(num), "%d", view.m_waveNumber);

    // Numeric win target: one plain centered string.
    if (view.m_victoryWave > 0) {
        DrawTextCenteredX(TextFormat("Wave: %s / %d", num, view.m_victoryWave),
                          centerX, m_textY, font, RAYWHITE, Text::Face::Mono);
        return;
    }

    // Endless: the target is the infinity glyph. The default font has no U+221E, so it is drawn by
    // hand; the readout is laid out in measured segments so the whole thing stays centered.
    const char* left = TextFormat("Wave: %s / ", num);
    float glyphH = static_cast<float>(font);
    float glyphW = glyphH * 1.2f;
    float gap = Scaled(2.0f);

    int leftW = Text::Measure(left, font, Text::Face::Mono);
    float totalW = static_cast<float>(leftW) + gap + glyphW;
    float startX = static_cast<float>(centerX) - totalW / 2.0f;

    Text::Draw(left, static_cast<int>(startX), m_textY, font, RAYWHITE, Text::Face::Mono);
    float glyphX = startX + static_cast<float>(leftW) + gap;
    DrawInfinity(glyphX, static_cast<float>(m_textY) + glyphH / 2.0f, glyphH, Color{120, 180, 220, 255});
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
