#include <core/input_manager.hpp>
#include <core/renderer.hpp>
#include <core/jsonio.hpp>
#include <optional>

void InputManager::Update(const Renderer& renderer) {
    m_mouseConsumed = false;
    m_virtualMouse = renderer.GetVirtualMouse();
}

void InputManager::AddAction(const std::string& action, KeyboardKey key) {
    m_bindings[action] = key;
}

void InputManager::AddAction(const std::string& action, MouseButton button) {
    m_bindings[action] = button;
}

bool InputManager::IsPressed(const std::string& action) const {
    auto it = m_bindings.find(action);
    if (it == m_bindings.end()) return false;
    return std::visit([](auto&& b) -> bool {
        using T = std::decay_t<decltype(b)>;
        if constexpr (std::is_same_v<T, KeyboardKey>)
            return b != KEY_NULL && IsKeyPressed(b);
        else
            return IsMouseButtonPressed(b);
    }, it->second);
}

bool InputManager::IsDown(const std::string& action) const {
    auto it = m_bindings.find(action);
    if (it == m_bindings.end()) return false;
    return std::visit([](auto&& b) -> bool {
        using T = std::decay_t<decltype(b)>;
        if constexpr (std::is_same_v<T, KeyboardKey>)
            return b != KEY_NULL && IsKeyDown(b);
        else
            return IsMouseButtonDown(b);
    }, it->second);
}

bool InputManager::IsReleased(const std::string& action) const {
    auto it = m_bindings.find(action);
    if (it == m_bindings.end()) return false;
    return std::visit([](auto&& b) -> bool {
        using T = std::decay_t<decltype(b)>;
        if constexpr (std::is_same_v<T, KeyboardKey>)
            return b != KEY_NULL && IsKeyReleased(b);
        else
            return IsMouseButtonReleased(b);
    }, it->second);
}

float InputManager::GetMouseWheelDelta() const {
    return GetMouseWheelMove();
}

void InputManager::ConsumeMouseInput() {
    m_mouseConsumed = true;
}

static std::optional<MouseButton> ParseMouseButton(const std::string& name) {
    if (name == "MOUSE_LEFT") return MOUSE_LEFT_BUTTON;
    if (name == "MOUSE_RIGHT") return MOUSE_RIGHT_BUTTON;
    if (name == "MOUSE_MIDDLE") return MOUSE_MIDDLE_BUTTON;
    return std::nullopt;
}

static KeyboardKey ParseKey(const std::string& name) {
    static const std::unordered_map<std::string, KeyboardKey> kMap = {
        {"A", KEY_A}, {"B", KEY_B}, {"C", KEY_C}, {"D", KEY_D}, {"E", KEY_E},
        {"F", KEY_F}, {"G", KEY_G}, {"H", KEY_H}, {"I", KEY_I}, {"J", KEY_J},
        {"K", KEY_K}, {"L", KEY_L}, {"M", KEY_M}, {"N", KEY_N}, {"O", KEY_O},
        {"P", KEY_P}, {"Q", KEY_Q}, {"R", KEY_R}, {"S", KEY_S}, {"T", KEY_T},
        {"U", KEY_U}, {"V", KEY_V}, {"W", KEY_W}, {"X", KEY_X}, {"Y", KEY_Y},
        {"Z", KEY_Z},
        {"0", KEY_ZERO}, {"1", KEY_ONE}, {"2", KEY_TWO}, {"3", KEY_THREE},
        {"4", KEY_FOUR}, {"5", KEY_FIVE}, {"6", KEY_SIX}, {"7", KEY_SEVEN},
        {"8", KEY_EIGHT}, {"9", KEY_NINE},
        {"ENTER", KEY_ENTER}, {"ESCAPE", KEY_ESCAPE}, {"SPACE", KEY_SPACE},
        {"GRAVE", KEY_GRAVE}, {"TAB", KEY_TAB},
        {"ARROW_LEFT", KEY_LEFT}, {"ARROW_RIGHT", KEY_RIGHT},
        {"ARROW_UP", KEY_UP}, {"ARROW_DOWN", KEY_DOWN},
        {"LEFT_SHIFT", KEY_LEFT_SHIFT}, {"LEFT_CTRL", KEY_LEFT_CONTROL},
        {"LEFT_ALT", KEY_LEFT_ALT},
        {"F1", KEY_F1}, {"F2", KEY_F2}, {"F3", KEY_F3}, {"F4", KEY_F4},
        {"F5", KEY_F5}, {"F6", KEY_F6}, {"F7", KEY_F7}, {"F8", KEY_F8},
    };
    auto it = kMap.find(name);
    return it != kMap.end() ? it->second : KEY_NULL;
}

void InputManager::Load(JsonIO& jsonio) {
    if (!jsonio.Exists("config/keybindings.json"))
        return;
    auto j = jsonio.Load("config/keybindings.json");
    for (auto& [action, value] : j.items()) {
        std::string name = value.get<std::string>();
        if (auto mb = ParseMouseButton(name))
            AddAction(action, *mb);
        else
            AddAction(action, ParseKey(name));
    }
}
