#pragma once
#include <raylib.h>
#include <string>

struct WidgetStyle {
    Color m_bgNormal; // default background
    Color m_bgHovered; // button hover
    Color m_bgInput; // text input background
    Color m_bgActive; // toggle on state
    Color m_border; // normal border
    Color m_borderSel; // button selected border
    Color m_accent; // slider fill, focused input border
    Color m_text; // toggle label, input text
    float m_borderWidth;
    float m_borderWidthActive; // selected button, focused input
};

// Default palette for widgets.
inline const WidgetStyle kDefaultStyle{
    .m_bgNormal   = {40,  40,  40,  255},
    .m_bgHovered  = {65,  65,  65,  255},
    .m_bgInput    = {30,  30,  30,  255},
    .m_bgActive   = {80,  180, 80,  255},
    .m_border     = {80,  80,  80,  255},
    .m_borderSel  = {255, 180, 0,   255},
    .m_accent     = {100, 149, 237, 255},
    .m_text       = {255, 255, 255, 255},
    .m_borderWidth       = 1.0f,
    .m_borderWidthActive = 2.0f,
};

// Muted palette for non-interactive / unavailable widgets.
inline const WidgetStyle kDisabledStyle {
    .m_bgNormal   = {30,  30,  30,  200},
    .m_bgHovered  = {30,  30,  30,  200},
    .m_bgInput    = {20,  20,  20,  200},
    .m_bgActive   = {40,  40,  40,  200},
    .m_border     = {60,  60,  60,  255},
    .m_borderSel  = {60,  60,  60,  255},
    .m_accent     = {60,  60,  60,  255},
    .m_text       = {100, 100, 100, 255},
    .m_borderWidth       = 1.0f,
    .m_borderWidthActive = 1.0f,
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
    Rectangle m_rect = {};
    float m_value = 0.0f;
    float m_min = 0.0f;
    float m_max = 1.0f;

    void Update(Vector2 mouse, bool held);
    bool IsDragging() const { return m_dragging; }
    void Draw(const WidgetStyle& style = kDefaultStyle) const;

private:
    bool m_dragging = false;
};

struct Toggle {
    Rectangle m_rect = {};
    std::string m_label;
    bool m_value = false;

    void Update(Vector2 mouse, bool pressed);
    bool IsClicked() const { return m_clicked; }
    void Draw(const WidgetStyle& style = kDefaultStyle) const;

private:
    bool m_clicked = false;
};

struct TextInput {
    Rectangle m_rect = {};
    std::string m_text;
    int m_maxLength = 64;

    void Update(Vector2 mouse, bool pressed);
    bool IsFocused() const { return m_focused; }
    void Draw(const WidgetStyle& style = kDefaultStyle) const;

private:
    bool m_focused = false;
};

struct ProgressBar {
    Rectangle m_rect = {};
    float m_value = 0.0f;
    float m_max = 1.0f;

    void Draw(const WidgetStyle& style = kDefaultStyle) const;
};
