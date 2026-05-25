#pragma once

#include <raylib.h>
#include <unordered_map>
#include <string>
#include <variant>

class Screen;
class JsonStore;

class Input {
public:
    Input() = default;

    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    void Update(const Screen& renderer);
    void Load(JsonStore& jsonio);

    bool IsPressed(const std::string& action) const;
    bool IsDown(const std::string& action) const;
    bool IsReleased(const std::string& action) const;

    float GetMouseWheelDelta() const;
    Vector2 GetMousePosition() const { return m_virtualMouse; }
    Vector2 GetWorldMousePosition(const Camera2D& camera) const { return GetScreenToWorld2D(m_virtualMouse, camera); }

    void ConsumeMouseInput();
    bool IsMouseInputConsumed() const { return m_mouseConsumed; }

    void AddAction(const std::string& action, KeyboardKey key);
    void AddAction(const std::string& action, MouseButton button);

private:
    using Binding = std::variant<KeyboardKey, MouseButton>;
    std::unordered_map<std::string, Binding> m_bindings;
    Vector2 m_virtualMouse = {};
    bool m_mouseConsumed = false;
};
