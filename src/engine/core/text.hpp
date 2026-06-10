#pragma once

#include <raylib.h>

// Global text drawing facade: GPU vector text (TextRenderer) using the
// VictorMono font embedded in the binary. Drop-in signatures for raylib's
// DrawText/MeasureText so call sites need no other changes. Falls back to
// raylib's bitmap font where the GPU path is unavailable (e.g. web builds).
namespace Text {

// Creates the renderer and loads the embedded font; warns and arms the
// raylib fallback on failure. Requires an initialized window.
void Init();

// Releases GPU resources. Must run before CloseWindow(); Draw/Measure
// fall back to raylib afterwards.
void Shutdown();

// Draws UTF-8 text (multi-line via '\n') with its top-left corner at (x, y).
void Draw(const char* text, int x, int y, int fontSize, Color color);

// Width in pixels of the widest line of text at fontSize.
int Measure(const char* text, int fontSize);

} // namespace Text
