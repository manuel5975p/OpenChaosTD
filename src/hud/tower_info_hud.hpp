#pragma once

#include <hud/hud.hpp>
#include <hud/button.hpp>
#include <world/tower.hpp>
#include <raylib.h>
#include <string>
#include <vector>

class Game;

class TowerInfoHUD : public HUD {
public:
    void Build(Game& game);
    void SetAnchor(Vector2 screenPos, int screenW, int screenH, const Tower& tower, bool showSell = true);

    void ProcessInput(Game& game);
    void Draw(Game& game, const Tower& tower);

    bool WasSellRequested();

private:
    float m_panelW    = 160.0f;
    float m_margin    =   8.0f;
    float m_lineH     =  15.0f;
    float m_descLineH =  13.0f;
    float m_headerH   =  20.0f;
    float m_sellH     =  22.0f;
    float m_sellGap   =   6.0f;
    float m_anchorGap =  20.0f;
    int   m_fontSm     = 11;
    int   m_fontDesc   = 10;
    int   m_fontHeader = 14;

    Button m_sellBtn;
    bool m_sellRequested = false;
    bool m_showSell = true;
    std::vector<std::string> m_descLines;
};
