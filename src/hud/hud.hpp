#pragma once
#include <raylib.h>

class Game;
class Input;

// One-shot flag: raised on an event, consumed exactly once.
struct HudSignal {
    void Raise() { m_pending = true; }
    bool Consume() { bool p = m_pending; m_pending = false; return p; }
private:
    bool m_pending = false;
};

void DrawTextCenteredX(const char* text, int centerX, int y, int fontSize, Color color);

// Base for every HUD component: uniform lifecycle, shared scaling and panel helpers.
// Input and draw are skipped automatically while hidden.
class HUD {
public:
    virtual ~HUD() = default;

    virtual void Update(Game& /*game*/, float /*dt*/) {}

    // Called once on state enter; concrete HUDs call this then do their own layout.
    void Build(float scale) { m_scale = scale; }

    void ProcessInput(Game& game) { if (m_visible) OnProcessInput(game); }
    void Draw(Game& game)         { if (m_visible) OnDraw(game); }

    void Show() { m_visible = true; }
    void Hide() { m_visible = false; }

protected:
    virtual void OnProcessInput(Game& /*game*/) {}
    virtual void OnDraw(Game& /*game*/) {}

    float Scaled(float base) const { return base * m_scale; }
    int   ScaledInt(float base) const { return static_cast<int>(base * m_scale); }

    void DrawPanelBackground(unsigned char alpha, bool border = false) const;
    void ConsumePanelClick(Input& input) const;
    void ClampPanelToScreen(int screenW, int screenH);

    float m_scale = 1.0f;
    bool m_visible = true;
    Rectangle m_panelRect = {};
};
