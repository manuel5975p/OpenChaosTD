#pragma once

#include <cmath>
#include <raylib.h>
#include <string>

class Enemy{
public:
    std::string m_name;
    Vector2 m_position;
    float m_maxhealth;
    float m_health;
    float m_speed;

    int m_pathIndex;
    int m_waypointIndex;
    Vector2 m_target = {MAXFLOAT, MAXFLOAT};
    bool m_reachedCore = false;
};