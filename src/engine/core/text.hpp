#pragma once

#include <raylib.h>

class FileStore;

// Global text drawing facade: GPU vector text (TextRenderer) drawn in semantic
// "kinds", each mapped to a font by config/fonts.toml (see FontConfig). Drop-in
// signatures for raylib's DrawText/MeasureText so call sites need no other
// changes; the optional Kind selects the typeface. Falls back to raylib's
// bitmap font where the GPU path is unavailable (e.g. web builds).
namespace Text {

// Semantic text roles. Each maps to a font name in config/fonts.toml; several
// kinds may resolve to the same font. Body is the default for general UI copy.
enum class Kind { Title, Heading, Body, Label, Number, Button, Tooltip };

// Number of Kind values — indexes the per-kind font table.
inline constexpr int KindCount = 7;

// Creates the renderer, reads config/fonts.toml via fileStore, and resolves each
// Kind to a font (file override in assets/fonts/ or an embedded default); warns
// and arms the raylib fallback on failure. Requires an initialized window.
void Init(FileStore& fileStore);

// Releases GPU resources. Must run before CloseWindow(); Draw/Measure
// fall back to raylib afterwards.
void Shutdown();

// Draws UTF-8 text (multi-line via '\n') with its top-left corner at (x, y).
void Draw(const char* text, int x, int y, int fontSize, Color color, Kind kind = Kind::Body);

// Width in pixels of the widest line of text at fontSize.
int Measure(const char* text, int fontSize, Kind kind = Kind::Body);

} // namespace Text
