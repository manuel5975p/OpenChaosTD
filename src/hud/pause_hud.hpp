#pragma once

#include <hud/hud.hpp>
#include <engine/features/ui_widgets.hpp>
#include <raylib.h>

class Game;

// Centered pause overlay: dims the screen so the world stays visible behind it and offers
// Resume / Restart / Main Menu. Hidden by default; PlayingState shows it while the simulation
// is paused. Each button raises a one-shot signal that PlayingState consumes.
class PauseHUD : public HUD {
public:
    void Build(Game& game);

    bool WasResumeRequested()   { return m_resumeSignal.Consume(); }
    bool WasRestartRequested()  { return m_restartSignal.Consume(); }
    bool WasMainMenuRequested() { return m_mainMenuSignal.Consume(); }

protected:
    void OnProcessInput(Game& game) override;
    void OnDraw(Game& game) override;

private:
    Button m_resumeBtn;
    Button m_restartBtn;
    Button m_mainMenuBtn;
    HudSignal m_resumeSignal;
    HudSignal m_restartSignal;
    HudSignal m_mainMenuSignal;
};
