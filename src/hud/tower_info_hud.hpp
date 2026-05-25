#pragma once

#include <engine/ui/hud.hpp>
#include <engine/ui/button.hpp>
#include <world/tower.hpp>
#include <raylib.h>
#include <string>
#include <vector>

class Game;

class TowerInfoHUD : public HUD {
public:
    void Build(Game& game);

    // Point the panel at a tower, position it near a screen anchor, and show it
    void SetTarget(Game& game, const Tower& tower, Vector2 screenPos, bool showSell);

    bool WasSellRequested() { return m_sellSignal.Consume(); }

protected:
    void OnProcessInput(Game& game) override;
    void OnDraw(Game& game) override;

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

    const Tower* m_target = nullptr;
    Button m_sellBtn;
    HudSignal m_sellSignal;
    bool m_showSell = true;
    std::vector<std::string> m_descLines;
};
