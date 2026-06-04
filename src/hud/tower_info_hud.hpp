#pragma once

#include <hud/hud.hpp>
#include <engine/features/ui_widgets.hpp>
#include <world/tower.hpp>
#include <raylib.h>
#include <string>
#include <vector>

class Game;

class TowerInfoHUD : public HUD {
public:
    void Build(Game& game);

    // Point the panel at a tower, position it near a screen anchor, and show it.
    // interactive = a real selected tower (shows config buttons) vs a hover preview.
    void SetTarget(Game& game, const Tower& tower, Vector2 screenPos, bool interactive);

    bool WasSellRequested() { return m_sellSignal.Consume(); }
    bool WasTargetingCycleRequested() { return m_targetSignal.Consume(); }
    bool WasUpgradeRequested() { return m_upgradeSignal.Consume(); }

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
    Button m_targetBtn;
    Button m_upgradeBtn;
    HudSignal m_sellSignal;
    HudSignal m_targetSignal;
    HudSignal m_upgradeSignal;
    bool m_showSell = true;
    bool m_sellEnabled = true;       // sellable only between waves; greyed out during a wave
    bool m_showTargeting = false;
    bool m_showUpgrade = false;
    bool m_upgradeReady = false;    // affordable and not yet max level
    bool m_hasNextUpgrade = false;  // an unpurchased upgrade level exists
    std::vector<std::string> m_descLines;
    std::vector<DescLine> m_upgradePreview; // delta lines for the next upgrade

    void DrawUpgradeTooltip(Game& game);
};
