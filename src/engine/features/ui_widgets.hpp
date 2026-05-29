#pragma once
#include <raylib.h>
#include <string>

struct WidgetStyle {
    Color bgNormal; // default background
    Color bgHovered; // button hover
    Color bgInput; // text input background
    Color bgActive; // toggle on state
    Color border; // normal border
    Color borderSel; // button selected border
    Color accent; // slider fill, focused input border
    Color text; // toggle label, input text
    float borderWidth;
    float borderWidthActive; // selected button, focused input
};

// Default palette for widgets.
inline const WidgetStyle kDefaultStyle{
    .bgNormal   = {40,  40,  40,  255},
    .bgHovered  = {65,  65,  65,  255},
    .bgInput    = {30,  30,  30,  255},
    .bgActive   = {80,  180, 80,  255},
    .border     = {80,  80,  80,  255},
    .borderSel  = {255, 180, 0,   255},
    .accent     = {100, 149, 237, 255},
    .text       = {255, 255, 255, 255},
    .borderWidth       = 1.0f,
    .borderWidthActive = 2.0f,
};

// Muted palette for non-interactive / unavailable widgets.
inline const WidgetStyle kDisabledStyle {
    .bgNormal   = {30,  30,  30,  200},
    .bgHovered  = {30,  30,  30,  200},
    .bgInput    = {20,  20,  20,  200},
    .bgActive   = {40,  40,  40,  200},
    .border     = {60,  60,  60,  255},
    .borderSel  = {60,  60,  60,  255},
    .accent     = {60,  60,  60,  255},
    .text       = {100, 100, 100, 255},
    .borderWidth       = 1.0f,
    .borderWidthActive = 1.0f,
};

struct Button {
    Rectangle m_rect = {};
    std::string m_label;

    void Update(Vector2 mouse, bool pressed);
    bool IsClicked() const { return m_clicked; }
    bool IsHovered() const { return m_hovered; }
    void Draw(bool selected = false, const WidgetStyle& style = kDefaultStyle) const;
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
    void Draw(const WidgetStyle& style = kDefaultStyle) const;

private:
    bool m_dragging = false;
};

struct Toggle {
    Rectangle rect = {};
    std::string label;
    bool value = false;

    void Update(Vector2 mouse, bool pressed);
    bool IsClicked() const { return m_clicked; }
    void Draw(const WidgetStyle& style = kDefaultStyle) const;

private:
    bool m_clicked = false;
};

struct TextInput {
    Rectangle rect = {};
    std::string text;
    int maxLength = 64;

    void Update(Vector2 mouse, bool pressed);
    bool IsFocused() const { return m_focused; }
    void Draw(const WidgetStyle& style = kDefaultStyle) const;

private:
    bool m_focused = false;
};

struct ProgressBar {
    Rectangle rect = {};
    float value = 0.0f;
    float max = 1.0f;

    void Draw(const WidgetStyle& style = kDefaultStyle) const;
};
