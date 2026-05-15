#pragma once

#include <core/button.hpp>
#include <world/tower.hpp>
#include <raylib.h>

class Game;

class TowerInfoHUD {
public:
    // Call once per frame (in ProcessInput) to position the panel above the selected tower.
    // Needs the tower to compute the correct panel height from its actual module count.
    void SetAnchor(Vector2 screenPos, int screenW, int screenH, const Tower& tower);

    void ProcessInput(Game& game);
    void Draw(Game& game, const Tower& tower);

    // Returns true once when sell is clicked, then resets — caller handles gold/removal
    bool WasSellRequested();

private:
    static constexpr float PANEL_W = 160.0f;
    static constexpr float MARGIN  = 8.0f;
    static constexpr float LINE_H  = 15.0f;  // vertical step between stat rows
    static constexpr int   FONT_SM = 11;

    Rectangle m_panelRect    = {};
    float     m_panelH       = 0.0f;  // computed from tower content in SetAnchor
    Button    m_sellBtn;
    bool      m_sellRequested = false;
};
