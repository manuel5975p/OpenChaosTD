#include "text.hpp"

#include "text_renderer.hpp"

#include <cstddef>
#include <memory>

// Generated at build time from VictorMono-Regular.ttf (cmake/embed_resource.cmake)
extern const unsigned char gVictorMonoTtf[];
extern const std::size_t gVictorMonoTtfSize;

namespace {

std::unique_ptr<TextRenderer> sRenderer;
TextRenderer::FontId sFont = 0;

} // namespace

namespace Text {

void Init() {
    auto renderer = TextRenderer::Create();
    if (!renderer) {
        TraceLog(LOG_WARNING, "TEXT: %s — falling back to raylib DrawText", renderer.error().c_str());
        return;
    }
    auto font = (*renderer)->LoadFontFromMemory(gVictorMonoTtf, gVictorMonoTtfSize);
    if (!font) {
        TraceLog(LOG_WARNING, "TEXT: embedded font failed (%s) — falling back to raylib DrawText",
                 font.error().c_str());
        return;
    }
    sRenderer = std::move(*renderer);
    sFont = *font;
}

void Shutdown() {
    sRenderer.reset();
}

void Draw(const char* text, int x, int y, int fontSize, Color color) {
    if (!sRenderer) {
        ::DrawText(text, x, y, fontSize, color);
        return;
    }
    sRenderer->DrawText(sFont, text,
                        {static_cast<float>(x), static_cast<float>(y)},
                        static_cast<float>(fontSize), color);
}

int Measure(const char* text, int fontSize) {
    if (!sRenderer)
        return ::MeasureText(text, fontSize);
    const Vector2 size = sRenderer->MeasureText(sFont, text, static_cast<float>(fontSize));
    return static_cast<int>(size.x + 0.5f);
}

} // namespace Text
