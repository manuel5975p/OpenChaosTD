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

private:
    std::vector<Button> m_buttons;
    std::string m_selectedTower;
    Rectangle m_panelRect = {};
};
