#pragma once

#include <hud/hud.hpp>
#include <hud/hud_views.hpp>
#include <raylib.h>

class Input;
class Resources;

// Compact right-side panel summarising the upcoming wave: total budget plus a card per enemy type
// showing its sprite, current upgrade level, and fully-upgraded stats. Content comes from a
// read-only WaveView built by PlayingState. Hidden by default; toggled by the StatusHUD "Waves"
// button or the hotkey.
class WaveHUD : public HUD {
public:
    void Build(float scale, int screenW);

    // Flip visibility — driven by the Waves button signal and the WaveInfo hotkey.
    void Toggle() { if (m_visible) Hide(); else Show(); }

    void ProcessInput(Input& input);
    void Draw(const WaveView& view, Resources& assets);

private:
    float m_panelW    = 200.0f;
    float m_margin    =   8.0f;
    float m_lineH     =  14.0f;
    float m_headerH   =  20.0f;
    float m_cardGap   =   6.0f; // vertical space between cards
    float m_cardPad   =   6.0f; // inner padding of a card
    float m_iconSize  =  44.0f; // square sprite area inside a card
    float m_topOffset =  42.0f; // pushes the panel below the top status bar
    int   m_fontSm     = 11;
    int   m_fontHeader = 14;
    int   m_screenW = 0;
};
