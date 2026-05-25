#include <engine/ui/ui_util.hpp>

void DrawTextCenteredX(const char* text, int centerX, int y, int fontSize, Color color) {
    int width = MeasureText(text, fontSize);
    DrawText(text, centerX - width / 2, y, fontSize, color);
}
