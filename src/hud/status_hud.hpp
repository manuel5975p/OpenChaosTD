#pragma once

#include <hud/hud.hpp>
#include <engine/features/ui_widgets.hpp>
#include <raylib.h>

class StatusHUD : public HUD {
public:
    void Build(Game& game);

    void SetAutoSpawn(bool autoSpawn) { m_autoSpawn = autoSpawn; }
    void SetSpeed(int speed) { m_speed = speed; m_speedBtn.m_label = TextFormat("%dx", speed); }

    bool WasWaveRequested() { return m_waveSignal.Consume(); }
    bool WasAutoToggled()   { return m_autoSignal.Consume(); }
    bool WasSpeedToggled()  { return m_speedSignal.Consume(); }

protected:
    void OnProcessInput(Game& game) override;
    void OnDraw(Game& game) override;

private:
    Button m_startWaveBtn;
    Button m_autoBtn;
    Button m_speedBtn;
    HudSignal m_waveSignal;
    HudSignal m_autoSignal;
    HudSignal m_speedSignal;
    int m_textY = 10;
    bool m_autoSpawn = false;
    int m_speed = 1;
};
