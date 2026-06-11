#pragma once

#include <raylib.h>
#include <engine/core/text.hpp>
#include <algorithm>
#include <string>

// Primitive drawing helpers shared by states and HUDs. These operate on raw
// virtual coordinates (no HUD scaling) and depend only on raylib + the Text
// facade, so they are safe to include from any UI layer.

// Draw text horizontally centered on centerX, with its top at y.
inline void DrawCenteredText(const char* text, float centerX, float y, int fontSize, Color color,
                             Text::Face face = Text::Face::Prose) {
    int w = Text::Measure(text, fontSize, face);
    Text::Draw(text, static_cast<int>(centerX - w / 2.0f), static_cast<int>(y), fontSize, color, face);
}

// Vertically center a label inside a row of the given height, left-aligned at x.
inline void DrawLabelInRow(const char* text, float x, float rowY, float rowH, int fontSize, Color color,
                           Text::Face face = Text::Face::Prose) {
    Text::Draw(text, static_cast<int>(x), static_cast<int>(rowY + (rowH - fontSize) / 2.0f), fontSize, color, face);
}

// Trim text with a trailing ellipsis so it fits within maxWidth pixels.
inline std::string TruncateToWidth(const std::string& text, int fontSize, float maxWidth,
                                   Text::Face face = Text::Face::Prose) {
    if (Text::Measure(text.c_str(), fontSize, face) <= maxWidth) return text;
    std::string out = text;
    while (!out.empty() && Text::Measure((out + "...").c_str(), fontSize, face) > maxWidth)
        out.pop_back();
    return out + "...";
}

// Draw a texture aspect-fitted (letterboxed) inside region, centered. No-op on an empty texture.
inline void DrawTextureFitted(const Texture2D& tex, Rectangle region) {
    if (tex.id == 0 || tex.width <= 0 || tex.height <= 0) return;
    float scale = std::min(region.width / tex.width, region.height / tex.height);
    float w = tex.width * scale;
    float h = tex.height * scale;
    Rectangle dst = {region.x + (region.width - w) / 2.0f,
                     region.y + (region.height - h) / 2.0f, w, h};
    Rectangle src = {0.0f, 0.0f, static_cast<float>(tex.width), static_cast<float>(tex.height)};
    DrawTexturePro(tex, src, dst, {0.0f, 0.0f}, 0.0f, WHITE);
}

// Exact component-wise equality for raylib Color (which has no built-in operator==).
inline bool ColorEquals(Color a, Color b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}
