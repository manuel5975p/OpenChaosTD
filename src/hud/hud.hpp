#pragma once
#include <raylib.h>
#include <engine/core/text.hpp>

class Input;

// One-shot flag: raised on an event, consumed exactly once.
struct HudSignal {
    void Raise() { m_pending = true; }
    bool Consume() { bool p = m_pending; m_pending = false; return p; }
private:
    bool m_pending = false;
};

void DrawTextCenteredX(const char* text, int centerX, int y, int fontSize, Color color,
                       Text::Face face = Text::Face::Prose);

// Base for every HUD component: shared scaling, panel helpers, and visibility state. Concrete
// HUDs expose their own typed ProcessInput/Draw methods fed by read-only views (see hud_views.hpp)
// and an Input& for clicks — they never receive Game or query gameplay state directly.
class HUD {
public:
    virtual ~HUD() = default;

    // Called once on state enter; concrete HUDs call this then do their own layout.
    void Build(float scale) { m_scale = scale; }

    void Show() { m_visible = true; }
    void Hide() { m_visible = false; }
    bool IsVisible() const { return m_visible; }

protected:
    float Scaled(float base) const { return base * m_scale; }
    int   ScaledInt(float base) const { return static_cast<int>(base * m_scale); }

    void DrawPanelBackground(unsigned char alpha, bool border = false) const;
    void ConsumePanelClick(Input& input) const;
    void ClampPanelToScreen(int screenW, int screenH);

    float m_scale = 1.0f;
    bool m_visible = true;
    Rectangle m_panelRect = {};
};
