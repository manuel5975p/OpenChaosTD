#pragma once

#include <core/button.hpp>
#include <world/tower.hpp>
#include <raylib.h>
#include <string>
#include <vector>

class Game;

class TowerInfoHUD {
public:
    void SetAnchor(Vector2 screenPos, int screenW, int screenH, const Tower& tower, bool showSell = true);

    void ProcessInput(Game& game);
    void Draw(Game& game, const Tower& tower);

    bool WasSellRequested();

private:
    static constexpr float PANEL_W = 160.0f;
    static constexpr float MARGIN = 8.0f;
    static constexpr float LINE_H = 15.0f;
    static constexpr float DESC_LINE_H = 13.0f;
    static constexpr int FONT_SM = 11;
    static constexpr int FONT_DESC = 10;

    Rectangle m_panelRect = {};
    Button m_sellBtn;
    bool m_sellRequested = false;
    bool m_showSell = true;
    std::vector<std::string> m_descLines;
};
