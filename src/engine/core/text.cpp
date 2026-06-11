#include "text.hpp"

#include "text_renderer.hpp"

#include <cstddef>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// Generated at build time from the TTF in resources/fonts/ (cmake/embed_resource.cmake)
extern const unsigned char gVictorMonoTtf[];
extern const std::size_t gVictorMonoTtfSize;

namespace {

std::unique_ptr<TextRenderer> sRenderer;
TextRenderer::FontId sProseFont = 0;  // Victor Mono
TextRenderer::FontId sMonoFont = 0;   // Victor Mono

TextRenderer::FontId FontFor(Text::Face face) {
    return face == Text::Face::Mono ? sMonoFont : sProseFont;
}

} // namespace

namespace Text {

void Init() {
    auto renderer = TextRenderer::Create();
    if (!renderer) {
        TraceLog(LOG_WARNING, "TEXT: %s — falling back to raylib DrawText", renderer.error().c_str());
        return;
    }
    auto prose = (*renderer)->LoadFontFromMemory(gVictorMonoTtf, gVictorMonoTtfSize);
    auto mono = (*renderer)->LoadFontFromMemory(gVictorMonoTtf, gVictorMonoTtfSize);
    if (!prose || !mono) {
        TraceLog(LOG_WARNING, "TEXT: embedded font failed (%s) — falling back to raylib DrawText",
                 (!prose ? prose : mono).error().c_str());
        return;
    }
    sRenderer = std::move(*renderer);
    sProseFont = *prose;
    sMonoFont = *mono;
}

void Shutdown() {
    sRenderer.reset();
}

void Draw(const char* text, int x, int y, int fontSize, Color color, Face face) {
    if (!sRenderer) {
        ::DrawText(text, x, y, fontSize, color);
        return;
    }
    sRenderer->DrawText(FontFor(face), text,
                        {static_cast<float>(x), static_cast<float>(y)},
                        static_cast<float>(fontSize), color);
}

int Measure(const char* text, int fontSize, Face face) {
    if (!sRenderer)
        return ::MeasureText(text, fontSize);
    const Vector2 size = sRenderer->MeasureText(FontFor(face), text, static_cast<float>(fontSize));
    return static_cast<int>(size.x + 0.5f);
}

std::vector<std::string> Wrap(const std::string& text, float maxWidth, int fontSize, Face face) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string word, current;
    while (stream >> word) {
        std::string candidate = current.empty() ? word : current + " " + word;
        if (Measure(candidate.c_str(), fontSize, face) <= static_cast<int>(maxWidth))
            current = candidate;
        else {
            if (!current.empty()) lines.push_back(current);
            current = word;
        }
    }
    if (!current.empty()) lines.push_back(current);
    return lines;
}

} // namespace Text
