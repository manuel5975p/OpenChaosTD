#pragma once
#include <raylib.h>
#include <string>

struct Button {
    Rectangle m_rect = {};
    std::string m_label;

    bool IsHovered(Vector2 mousePos) const {
        return CheckCollisionPointRec(mousePos, m_rect);
    }

    bool IsClicked(Vector2 mousePos, bool mousePressed) const {
        return mousePressed && IsHovered(mousePos);
    }

    void Draw(Vector2 mousePos, bool selected = false) const {
        Color bg = IsHovered(mousePos) ? Color{65, 65, 65, 255} : Color{40, 40, 40, 255};
        DrawRectangleRec(m_rect, bg);
        Color border = selected ? Color{255, 180, 0, 255} : Color{80, 80, 80, 255};
        DrawRectangleLinesEx(m_rect, selected ? 2.0f : 1.0f, border);
    }
};
