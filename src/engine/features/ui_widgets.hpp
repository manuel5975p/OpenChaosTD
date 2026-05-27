#pragma once
#include <raylib.h>
#include <string>

struct Button {
    Rectangle m_rect = {};
    std::string m_label;

    void Update(Vector2 mouse, bool pressed);
    bool IsClicked() const { return m_clicked; }
    bool IsHovered() const { return m_hovered; }
    void Draw(bool selected = false) const;
    void DrawLabel(int fontSize, Color color) const;

private:
    bool m_hovered = false;
    bool m_clicked = false;
};

struct Slider {
    Rectangle rect = {};
    float value = 0.0f;
    float min = 0.0f;
    float max = 1.0f;

    void Update(Vector2 mouse, bool held);
    bool IsDragging() const { return m_dragging; }
    void Draw() const;

private:
    bool m_dragging = false;
};

struct Toggle {
    Rectangle rect = {};
    std::string label;
    bool value = false;

    void Update(Vector2 mouse, bool pressed);
    bool IsClicked() const { return m_clicked; }
    void Draw() const;

private:
    bool m_clicked = false;
};

struct TextInput {
    Rectangle rect = {};
    std::string text;
    int maxLength = 64;

    void Update(Vector2 mouse, bool pressed);
    bool IsFocused() const { return m_focused; }
    void Draw() const;

private:
    bool m_focused = false;
};
