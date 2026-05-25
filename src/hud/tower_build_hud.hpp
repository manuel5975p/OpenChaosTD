#pragma once

#include <engine/ui/hud.hpp>
#include <engine/ui/button.hpp>
#include <raylib.h>
#include <vector>
#include <string>

class TowerBuildHUD : public HUD {
public:
    void Build(Game& game);

    const std::string& GetSelectedTower() const { return m_selectedTower; }
    void ClearSelection() { m_selectedTower = ""; }

    // Returns the name of the button under mousePos, or "" if none
    const std::string& GetHoveredTower(Vector2 mousePos) const;
    // Returns the top-center screen position of the button under mousePos
    Vector2 GetHoveredButtonTopCenter(Vector2 mousePos) const;

protected:
    void OnProcessInput(Game& game) override;
    void OnDraw(Game& game) override;

private:
    std::vector<Button> m_buttons;
    std::string m_selectedTower;
};
