#include <engine/features/ui_widgets.hpp>
#include <algorithm>

// --- Button ---

void Button::Update(Vector2 mouse, bool pressed) {
    m_clicked = false;
    m_hovered = CheckCollisionPointRec(mouse, m_rect);
    if (m_hovered && pressed)
        m_clicked = true;
}

void Button::Draw(bool selected, const WidgetStyle& style) const {
    Color bg = m_hovered ? style.bgHovered : style.bgNormal;
    DrawRectangleRec(m_rect, bg);
    Color border = selected ? style.borderSel : style.border;
    DrawRectangleLinesEx(m_rect, selected ? style.borderWidthActive : style.borderWidth, border);
}

void Button::DrawLabel(int fontSize, Color color) const {
    if (m_label.empty()) return;
    int lw = MeasureText(m_label.c_str(), fontSize);
    DrawText(m_label.c_str(),
        static_cast<int>(m_rect.x + (m_rect.width - lw) / 2.0f),
        static_cast<int>(m_rect.y + (m_rect.height - fontSize) / 2.0f),
        fontSize, color);
}

// --- Slider ---

void Slider::Update(Vector2 mouse, bool held) {
    bool over = CheckCollisionPointRec(mouse, rect);
    m_dragging = held && (over || m_dragging);

    if (m_dragging && max > min) {
        float t = std::clamp((mouse.x - rect.x) / rect.width, 0.0f, 1.0f);
        value = min + t * (max - min);
    }
}

void Slider::Draw(const WidgetStyle& style) const {
    DrawRectangleRec(rect, style.bgNormal);
    DrawRectangleLinesEx(rect, style.borderWidth, style.border);

    if (max <= min) return;

    float t = (value - min) / (max - min);
    Rectangle filled = { rect.x, rect.y, rect.width * t, rect.height };
    DrawRectangleRec(filled, style.accent);

    // Knob
    float kx = rect.x + rect.width * t;
    DrawRectangle(static_cast<int>(kx - 3), static_cast<int>(rect.y) - 2,
        6, static_cast<int>(rect.height) + 4, style.text);
}

// --- Toggle ---

void Toggle::Update(Vector2 mouse, bool pressed) {
    m_clicked = false;
    if (CheckCollisionPointRec(mouse, rect) && pressed) {
        value = !value;
        m_clicked = true;
    }
}

void Toggle::Draw(const WidgetStyle& style) const {
    Color bg = value ? style.bgActive : style.bgNormal;
    DrawRectangleRec(rect, bg);
    DrawRectangleLinesEx(rect, style.borderWidth, style.border);

    if (!label.empty()) {
        int fontSize = static_cast<int>(rect.height * 0.65f);
        DrawText(label.c_str(),
            static_cast<int>(rect.x + rect.width + 6.0f),
            static_cast<int>(rect.y + (rect.height - fontSize) / 2.0f),
            fontSize, style.text);
    }
}

// --- TextInput ---

void TextInput::Update(Vector2 mouse, bool pressed) {
    if (pressed)
        m_focused = CheckCollisionPointRec(mouse, rect);

    if (!m_focused) return;

    int ch = GetCharPressed();
    while (ch > 0) {
        if (ch >= 32 && static_cast<int>(text.size()) < maxLength)
            text += static_cast<char>(ch);
        ch = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !text.empty())
        text.pop_back();

    if (IsKeyPressed(KEY_ENTER))
        m_focused = false;
}

// --- ProgressBar ---

void ProgressBar::Draw(const WidgetStyle& style) const {
    DrawRectangleRec(rect, style.bgNormal);
    DrawRectangleLinesEx(rect, style.borderWidth, style.border);

    if (max > 0.0f && value > 0.0f) {
        float t = std::clamp(value / max, 0.0f, 1.0f);
        Rectangle filled = { rect.x, rect.y, rect.width * t, rect.height };
        DrawRectangleRec(filled, style.accent);
    }
}

// --- TextInput ---

void TextInput::Draw(const WidgetStyle& style) const {
    DrawRectangleRec(rect, style.bgInput);
    Color border = m_focused ? style.accent : style.border;
    DrawRectangleLinesEx(rect, m_focused ? style.borderWidthActive : style.borderWidth, border);

    int fontSize = static_cast<int>(rect.height * 0.6f);
    float padding = rect.height * 0.2f;
    std::string display = text + (m_focused ? "_" : "");
    DrawText(display.c_str(),
        static_cast<int>(rect.x + padding),
        static_cast<int>(rect.y + (rect.height - fontSize) / 2.0f),
        fontSize, style.text);
}
