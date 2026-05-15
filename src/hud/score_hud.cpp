#include <hud/score_hud.hpp>
#include <game.hpp>
#include <raylib.h>

static constexpr float PANEL_H    = 36.0f;
static constexpr float BTN_H      = 24.0f;
static constexpr float BTN_WAVE_W = 90.0f;
static constexpr float BTN_AUTO_W = 48.0f;
static constexpr float MARGIN     = 6.0f;

void ScoreHUD::Build(Game& game) {
    float w = static_cast<float>(game.GetRenderer().GetGameWidth());

    m_panelRect = { 0.0f, 0.0f, w, PANEL_H };

    float btnY = (PANEL_H - BTN_H) / 2.0f;

    // "Start Wave" anchored to the right edge
    m_startWaveBtn.m_label = "Start Wave";
    m_startWaveBtn.m_rect  = { w - BTN_WAVE_W - MARGIN, btnY, BTN_WAVE_W, BTN_H };

    // "Auto" toggle sits immediately to the left of "Start Wave"
    m_autoBtn.m_label = "Auto";
    m_autoBtn.m_rect  = { w - BTN_WAVE_W - MARGIN - BTN_AUTO_W - MARGIN, btnY, BTN_AUTO_W, BTN_H };
}

void ScoreHUD::ProcessInput(Game& game) {
    const auto& data = game.GetGameData();
    Vector2 mousePos = game.GetInput().GetMousePosition();

    // Consume panel clicks so they don't bleed through to the world
    if (game.GetInput().IsPressed("Select") && CheckCollisionPointRec(mousePos, m_panelRect))
        game.GetInput().ConsumeMouseInput();

    if (game.GetInput().IsPressed("Select")) {
        // Auto toggle is always clickable, even mid-wave
        if (m_autoBtn.IsClicked(mousePos, true))
            m_autoToggled = true;

        // Start wave only when no wave is running
        if (!data.waveActive && m_startWaveBtn.IsClicked(mousePos, true))
            m_waveRequested = true;
    }
}

bool ScoreHUD::WasWaveRequested() {
    if (!m_waveRequested) return false;
    m_waveRequested = false;
    return true;
}

bool ScoreHUD::WasAutoToggled() {
    if (!m_autoToggled) return false;
    m_autoToggled = false;
    return true;
}

void ScoreHUD::Draw(Game& game, bool autoSpawn) {
    const auto& data     = game.GetGameData();
    Vector2     mousePos = game.GetInput().GetMousePosition();

    DrawRectangleRec(m_panelRect, {20, 20, 20, 200});

    // Left side: lives and gold
    DrawText(TextFormat("Lives: %d", data.lives), static_cast<int>(MARGIN), 10, 16, RAYWHITE);
    DrawText(TextFormat("Gold: %d",  data.gold),  static_cast<int>(MARGIN) + 110, 10, 16, GOLD);

    // Center: wave number and elapsed time
    const char* waveStr;
    if (data.waveNumber == 0)
        waveStr = "Wave: --";
    else if (data.waveActive)
        waveStr = TextFormat("Wave: %d  |  %.1fs", data.waveNumber, data.waveTimer);
    else
        waveStr = TextFormat("Wave: %d  |  Done", data.waveNumber);

    int waveW   = MeasureText(waveStr, 16);
    int centerX = static_cast<int>(m_panelRect.width / 2.0f - waveW / 2.0f);
    DrawText(waveStr, centerX, 10, 16, RAYWHITE);

    // Auto toggle — highlighted when active
    m_autoBtn.Draw(mousePos, autoSpawn);
    m_autoBtn.DrawLabel(12, autoSpawn ? GOLD : RAYWHITE);

    // Start wave button — greyed out while a wave is running
    if (data.waveActive) {
        DrawRectangleRec(m_startWaveBtn.m_rect, {30, 30, 30, 200});
        DrawRectangleLinesEx(m_startWaveBtn.m_rect, 1.0f, {60, 60, 60, 255});
        m_startWaveBtn.DrawLabel(12, DARKGRAY);
    } else {
        m_startWaveBtn.Draw(mousePos);
        m_startWaveBtn.DrawLabel(12, RAYWHITE);
    }
}
