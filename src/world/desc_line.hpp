#pragma once

#include <string>
#include <raylib.h>

// One described line: a formatted text row plus the color it should render in.
// Every module's DescribeStats() appends zero or more of these (as does TowerUpgrade).
struct DescLine {
    std::string m_text;
    Color m_color;
};
