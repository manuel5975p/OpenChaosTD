#pragma once

#include <world/map.hpp>
#include <world/tower.hpp>
#include <world/enemy.hpp>
#include <world/attack.hpp>
#include <lib/dense_slotmap.hpp>
#include <vector>

class JsonIO;

struct GameData {
    int startingLives = 20;
    int startingGold = 150;

    int lives = 20;
    int gold = 150;
    int score = 0;
    bool victory = false;
    Map map;
    DenseSlotMap<Tower> towers;
    DenseSlotMap<Enemy> enemies;
    std::vector<Attack> attacks;

    void Load(JsonIO& jsonio);
    void Reset();
};
