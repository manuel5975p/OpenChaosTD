#pragma once

#include <hud/hud.hpp>
#include <engine/features/ui_widgets.hpp>
#include <raylib.h>

class WaveManager;

class StatusHUD : public HUD {
public:
    void Build(Game& game, const WaveManager& waveManager);

    void SetAutoSpawn(bool autoSpawn) { m_autoSpawn = autoSpawn; }
    void SetSpeed(int speed) { m_speed = speed; m_speedBtn.m_label = TextFormat("%dx", speed); }

    bool WasWaveRequested()    { return m_waveSignal.Consume(); }
    bool WasAutoToggled()      { return m_autoSignal.Consume(); }
    bool WasSpeedToggled()     { return m_speedSignal.Consume(); }
    bool WasWaveInfoToggled()  { return m_waveInfoSignal.Consume(); }

protected:
    void OnProcessInput(Game& game) override;
    void OnDraw(Game& game) override;

private:
    // Center readout: wave progress + win target + timer, drawn centered on centerX. Endless mode
    // shows the infinity glyph in place of a target wave.
    void DrawWaveReadout(Game& game, int centerX);
    // Hand-drawn infinity glyph (the default font lacks U+221E). Spans [x, x + 1.2*h] horizontally,
    // vertically centered on yMid.
    void DrawInfinity(float x, float yMid, float h, Color color) const;

    Button m_startWaveBtn;
    Button m_autoBtn;
    Button m_speedBtn;
    Button m_waveInfoBtn;
    HudSignal m_waveSignal;
    HudSignal m_autoSignal;
    HudSignal m_speedSignal;
    HudSignal m_waveInfoSignal;
    const WaveManager* m_waveManager = nullptr;
    int m_textY = 10;
    bool m_autoSpawn = false;
    int m_speed = 1;
};
