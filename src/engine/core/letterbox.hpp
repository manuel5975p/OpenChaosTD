#pragma once

#include <raylib.h>

// Pure DPI/letterbox geometry, split out from Screen so it can be unit-tested
// without a live GL context. All sizes are in framebuffer pixels; the mouse
// mapping additionally takes the window's DPI scale to bridge logical points
// (what GetMousePosition reports) and framebuffer pixels (what we draw in).

struct Letterbox {
    Rectangle destRect; // virtual rect placed on the framebuffer, centered
    float     scale;    // framebuffer-pixels per virtual unit
};

// Largest centered rect of the virtual aspect that fits the framebuffer.
// renderW/H and virtualW/H must be > 0.
Letterbox ComputeLetterbox(int renderW, int renderH, int virtualW, int virtualH);

// Maps a logical-point mouse position to virtual coordinates, clamped to the
// virtual bounds. dpiScale > 0; lb from ComputeLetterbox with the same virtual size.
Vector2 MapMouseToVirtual(Vector2 mouseLogical, float dpiScale,
                          const Letterbox& lb, int virtualW, int virtualH);
