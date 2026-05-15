#pragma once

#include <hud/hud.hpp>
#include <hud/button.hpp>
#include <raylib.h>

class Game;

class ScoreHUD : public HUD {
public:
    void Build(Game& game);
    void ProcessInput(Game& game);

    // autoSpawn drives the visual state of the auto toggle button
    void Draw(Game& game, bool autoSpawn);

    // Returns true once per wave request, then resets — caller handles spawning
    bool WasWaveRequested();

    // Returns true once when the auto toggle is clicked, then resets
    bool WasAutoToggled();

private:
    Button m_startWaveBtn;
    Button m_autoBtn;
    int m_textY = 10;
    bool m_waveRequested = false;
    bool m_autoToggled = false;
};
