#pragma once

#include <raylib.h>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <string_view>

// GPU-accelerated vector text renderer: HarfBuzz shaping + Slug coverage
// rendering via harfbuzz-gpu. Glyph outlines are encoded once into a GPU
// atlas and rasterized per-fragment, so text stays sharp at any scale and
// under any camera zoom — no pre-baked bitmap atlas, no FreeType.
//
// Fully independent of raylib's DrawText; respects the active raylib
// matrices (BeginMode2D camera, render texture viewport) and blend state
// is restored after each draw.
class TextRenderer {
public:
    // Opaque font handle; valid for the lifetime of the owning TextRenderer.
    using FontId = std::uint32_t;

    // Creates shader and GPU glyph atlas. Requires an active raylib window (GL 3.3).
    static std::expected<std::unique_ptr<TextRenderer>, std::string> Create();

    ~TextRenderer();

    // Non-copyable / non-movable — owns GL and HarfBuzz handles
    TextRenderer(const TextRenderer&)            = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    // Loads a TTF/OTF face directly through HarfBuzz (hb_face_create).
    // path must point to a valid font file; glyphs are encoded lazily on first use.
    std::expected<FontId, std::string> LoadFont(std::string_view path);

    // Loads a TTF/OTF face from a memory blob (e.g. font data embedded in the binary).
    // data must stay valid and unmodified for the renderer's lifetime; size > 0.
    std::expected<FontId, std::string> LoadFontFromMemory(const void* data, std::size_t size);

    // Shapes UTF-8 text (kerning, ligatures, multi-line via '\n') and draws it
    // with the top-left corner at position. font must come from LoadFont.
    void DrawText(FontId font, std::string_view text, Vector2 position, float fontSize, Color color);

    // Logical extents of the shaped text (width of widest line, total height).
    Vector2 MeasureText(FontId font, std::string_view text, float fontSize);

private:
    TextRenderer();

    std::expected<FontId, std::string> LoadFontBlob(struct hb_blob_t* blob);

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
