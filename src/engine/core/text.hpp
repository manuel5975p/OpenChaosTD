#pragma once

#include <raylib.h>

// Global text drawing facade: GPU vector text (TextRenderer) using two fonts
// embedded in the binary — EB Garamond for prose and Victor Mono for code and
// numeric readouts. Drop-in signatures for raylib's DrawText/MeasureText so
// call sites need no other changes; the optional Face selects the typeface.
// Falls back to raylib's bitmap font where the GPU path is unavailable
// (e.g. web builds).
namespace Text {

// Typeface selection. Prose (EB Garamond) is the default for UI copy; Mono
// (Victor Mono) is for code and numbers, where tabular figures read cleaner.
enum class Face { Prose, Mono };

// Creates the renderer and loads the embedded fonts; warns and arms the
// raylib fallback on failure. Requires an initialized window.
void Init();

// Releases GPU resources. Must run before CloseWindow(); Draw/Measure
// fall back to raylib afterwards.
void Shutdown();

// Draws UTF-8 text (multi-line via '\n') with its top-left corner at (x, y).
void Draw(const char* text, int x, int y, int fontSize, Color color, Face face = Face::Prose);

// Width in pixels of the widest line of text at fontSize.
int Measure(const char* text, int fontSize, Face face = Face::Prose);

} // namespace Text
