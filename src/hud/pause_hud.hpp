#pragma once

#include <hud/hud.hpp>
#include <engine/features/ui_widgets.hpp>
#include <raylib.h>

class Input;

// Centered pause overlay: dims the screen so the world stays visible behind it and offers
// Resume / Save / Load / Restart / Main Menu. Hidden by default; PlayingState shows it while the
// simulation is paused. Each button raises a one-shot signal that PlayingState consumes.
class PauseHUD : public HUD {
public:
    void Build(float scale, int screenW, int screenH);

    void ProcessInput(Input& input);
    void Draw();

    bool WasResumeRequested()   { return m_resumeSignal.Consume(); }
    bool WasSaveRequested()     { return m_saveSignal.Consume(); }
    bool WasLoadRequested()     { return m_loadSignal.Consume(); }
    bool WasRestartRequested()  { return m_restartSignal.Consume(); }
    bool WasMainMenuRequested() { return m_mainMenuSignal.Consume(); }

private:
    Button m_resumeBtn;
    Button m_saveBtn;
    Button m_loadBtn;
    Button m_restartBtn;
    Button m_mainMenuBtn;
    HudSignal m_resumeSignal;
    HudSignal m_saveSignal;
    HudSignal m_loadSignal;
    HudSignal m_restartSignal;
    HudSignal m_mainMenuSignal;
    int m_screenW = 0;
    int m_screenH = 0;
};
