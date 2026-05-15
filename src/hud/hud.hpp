#pragma once

#include <raylib.h>

class Game;

// Shared state and helpers for on-screen HUD components.
class HUD {
protected:
    // Cache the configured HUD scale into m_scale; call once from a derived Build()
    void LoadScale(Game& game);

    // Multiply an unscaled design value by the HUD scale
    float Scaled(float base) const { return base * m_scale; }
    int   ScaledInt(float base) const { return static_cast<int>(base * m_scale); }

    // Fill m_panelRect with the standard dark HUD background
    void DrawPanelBackground(unsigned char alpha, bool border = false) const;

    // Swallow a click landing on the panel so it doesn't bleed through to the world
    void ConsumePanelClick(Game& game, const char* action) const;

    float m_scale = 1.0f;
    Rectangle m_panelRect = {};
};
