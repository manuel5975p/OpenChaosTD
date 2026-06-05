#pragma once

#include <hud/hud.hpp>
#include <raylib.h>

class Game;
class WaveManager;

// Compact right-side panel summarising the upcoming wave: total budget plus each enemy type's
// count and key stats. Hidden by default; toggled by the StatusHUD "Waves" button or the hotkey.
class WaveHUD : public HUD {
public:
    void Build(Game& game, const WaveManager& waveManager);

    // Flip visibility — driven by the Waves button signal and the WaveInfo hotkey.
    void Toggle() { if (m_visible) Hide(); else Show(); }

protected:
    void OnProcessInput(Game& game) override;
    void OnDraw(Game& game) override;

private:
    float m_panelW    = 190.0f;
    float m_margin    =   8.0f;
    float m_lineH     =  14.0f;
    float m_rowGap    =   4.0f;
    float m_headerH   =  20.0f;
    float m_indent    =  10.0f;
    float m_topOffset =  42.0f; // pushes the panel below the top status bar
    int   m_fontSm     = 11;
    int   m_fontHeader = 14;

    const WaveManager* m_waveManager = nullptr;
};
