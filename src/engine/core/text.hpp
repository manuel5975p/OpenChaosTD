#pragma once

#include <raylib.h>
#include <string>
#include <vector>

// Global text drawing facade: GPU vector text (TextRenderer) using one font
// embedded in the binary — Victor Mono for both prose and code/numeric
// readouts. Drop-in signatures for raylib's DrawText/MeasureText so
// call sites need no other changes; the optional Face selects the typeface.
// Falls back to raylib's bitmap font where the GPU path is unavailable
// (e.g. web builds).
namespace Text {

// Typeface selection. Both faces use Victor Mono; the distinction is
// preserved for call sites that want to label their text semantically.
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

// Greedy word-wrap: split UTF-8 text into lines no wider than maxWidth pixels at fontSize.
std::vector<std::string> Wrap(const std::string& text, float maxWidth, int fontSize,
                              Face face = Face::Prose);

} // namespace Text
