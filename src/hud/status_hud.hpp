#pragma once

#include <engine/ui/hud.hpp>
#include <engine/ui/button.hpp>
#include <raylib.h>

class StatusHUD : public HUD {
public:
    void Build(Game& game);

    void SetAutoSpawn(bool autoSpawn) { m_autoSpawn = autoSpawn; }

    bool WasWaveRequested() { return m_waveSignal.Consume(); }
    bool WasAutoToggled()   { return m_autoSignal.Consume(); }

protected:
    void OnProcessInput(Game& game) override;
    void OnDraw(Game& game) override;

private:
    Button m_startWaveBtn;
    Button m_autoBtn;
    HudSignal m_waveSignal;
    HudSignal m_autoSignal;
    int m_textY = 10;
    bool m_autoSpawn = false;
};
