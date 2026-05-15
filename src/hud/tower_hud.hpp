#pragma once

#include <core/button.hpp>
#include <raylib.h>
#include <vector>
#include <string>

class Game;

class TowerHUD {
public:
    void Build(Game& game);
    void ProcessInput(Game& game);
    void Draw(Game& game);

    const std::string& GetSelectedTower() const { return m_selectedTower; }
    void ClearSelection() { m_selectedTower = ""; }

    // Returns the name of the button under mousePos, or "" if none
    const std::string& GetHoveredTower(Vector2 mousePos) const;
    // Returns the top-center screen position of the button under mousePos
    Vector2 GetHoveredButtonTopCenter(Vector2 mousePos) const;

private:
    std::vector<Button> m_buttons;
    std::string m_selectedTower;
    Rectangle m_panelRect = {};
};
