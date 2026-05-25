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

    // Draw the label centered inside the button rect
    void DrawLabel(int fontSize, Color color) const {
        if (m_label.empty()) return;
        int lw = MeasureText(m_label.c_str(), fontSize);
        DrawText(m_label.c_str(),
            static_cast<int>(m_rect.x + (m_rect.width  - lw) / 2.0f),
            static_cast<int>(m_rect.y + (m_rect.height - fontSize) / 2.0f),
            fontSize, color);
    }
};
