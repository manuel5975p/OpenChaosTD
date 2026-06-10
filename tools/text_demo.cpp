// Standalone demo for the harfbuzz-gpu Slug TextRenderer.
// Usage: text_demo [font.ttf] [--screenshot out.png]

#include <engine/core/text_renderer.hpp>

#include <raylib.h>

#include <cmath>
#include <cstdio>
#include <string>

int main(int argc, char** argv) {
    const char* fontPath = "/usr/share/fonts/TTF/DejaVuSans.ttf";
    const char* screenshotPath = nullptr;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--screenshot" && i + 1 < argc)
            screenshotPath = argv[++i];
        else
            fontPath = argv[i];
    }

    InitWindow(1280, 720, "harfbuzz-gpu Slug text demo");
    SetTargetFPS(60);

    auto renderer = TextRenderer::Create();
    if (!renderer) {
        std::fprintf(stderr, "TextRenderer::Create: %s\n", renderer.error().c_str());
        return 1;
    }
    auto font = (*renderer)->LoadFont(fontPath);
    if (!font) {
        std::fprintf(stderr, "LoadFont: %s\n", font.error().c_str());
        return 1;
    }

    Camera2D camera = {};
    camera.zoom = 1.0f;

    // Mirrors the game's Screen class: draw into a virtual-resolution texture.
    RenderTexture2D target = LoadRenderTexture(420, 80);

    int frame = 0;
    while (!WindowShouldClose()) {
        frame++;
        const float t = static_cast<float>(GetTime());
        camera.zoom = 1.0f + 0.5f*std::sin(t*0.7f);
        camera.target = {640, 360};
        camera.offset = {640, 360};
        camera.rotation = 10.0f*std::sin(t*0.3f);

        BeginTextureMode(target);
        ClearBackground(Color{30, 30, 40, 255});
        (*renderer)->DrawText(*font, "inside a RenderTexture2D", {10, 20}, 32, GOLD);
        EndTextureMode();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTextureRec(target.texture, {0, 0, 420, -80}, {820, 620}, WHITE);

        BeginMode2D(camera);
        (*renderer)->DrawText(*font, "Sharp vector text under any camera zoom.\nKerning AV To ff fi — shaped by HarfBuzz.",
                              {120, 200}, 48, DARKBLUE);
        (*renderer)->DrawText(*font, "tiny 12px line stays legible", {120, 340}, 12, BLACK);
        (*renderer)->DrawText(*font, "BIG", {120, 380}, 220, Color{200, 40, 40, 255});
        EndMode2D();

        const Vector2 m = (*renderer)->MeasureText(*font, "Measured box", 32);
        DrawRectangleLines(40, 40, static_cast<int>(m.x), static_cast<int>(m.y), GREEN);
        (*renderer)->DrawText(*font, "Measured box", {40, 40}, 32, DARKGREEN);
        DrawFPS(1180, 10);
        EndDrawing();

        if (screenshotPath && frame == 30) {
            TakeScreenshot(screenshotPath);
            break;
        }
    }

    UnloadRenderTexture(target);
    renderer->reset();  // release GL resources before the context goes away
    CloseWindow();
    return 0;
}
