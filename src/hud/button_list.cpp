#include <hud/button_list.hpp>

namespace Hud {

int ButtonList::Add(const std::string& label) {
    Item item;
    item.button.m_label = label;
    items.push_back(std::move(item));
    return static_cast<int>(items.size()) - 1;
}

void ButtonList::LayoutVertical(float x, float firstY, float w, float h, float spacing) {
    for (size_t i = 0; i < items.size(); i++)
        items[i].button.m_rect = { x, firstY + spacing * static_cast<float>(i), w, h };
}

void ButtonList::Update(Vector2 mouse, bool pressed, bool& clicked) {
    for (auto& item : items)
        item.button.Update(mouse, pressed);
    for (auto& item : items) {
        if (item.button.IsClicked()) {
            item.signal.Raise();
            clicked = true;
        }
    }
}

void ButtonList::Draw(int fontSize, Color labelColor) const {
    for (const auto& item : items) {
        item.button.Draw();
        item.button.DrawLabel(fontSize, labelColor);
    }
}

} // namespace Hud
