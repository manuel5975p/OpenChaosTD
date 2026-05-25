#pragma once
#include <raylib.h>

// One-shot flag: raised on an event, consumed exactly once.
struct HudSignal {
    void Raise() { m_pending = true; }
    bool Consume() { bool p = m_pending; m_pending = false; return p; }
private:
    bool m_pending = false;
};

void DrawTextCenteredX(const char* text, int centerX, int y, int fontSize, Color color);
