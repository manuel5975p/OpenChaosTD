#include <engine/core/letterbox.hpp>

#include <algorithm>

Letterbox ComputeLetterbox(int renderW, int renderH, int virtualW, int virtualH) {
    const float scaleX = static_cast<float>(renderW) / static_cast<float>(virtualW);
    const float scaleY = static_cast<float>(renderH) / static_cast<float>(virtualH);
    const float scale  = std::min(scaleX, scaleY); // fit inside, preserve aspect

    const float scaledW = static_cast<float>(virtualW) * scale;
    const float scaledH = static_cast<float>(virtualH) * scale;
    const float offsetX = (static_cast<float>(renderW) - scaledW) * 0.5f;
    const float offsetY = (static_cast<float>(renderH) - scaledH) * 0.5f;

    return { Rectangle{ offsetX, offsetY, scaledW, scaledH }, scale };
}

Vector2 MapMouseToVirtual(Vector2 mouseLogical, float dpiScale,
                          const Letterbox& lb, int virtualW, int virtualH) {
    // Logical points -> framebuffer pixels, then undo the letterbox transform.
    const float px = mouseLogical.x * dpiScale;
    const float py = mouseLogical.y * dpiScale;

    float vx = (px - lb.destRect.x) / lb.scale;
    float vy = (py - lb.destRect.y) / lb.scale;

    vx = std::clamp(vx, 0.0f, static_cast<float>(virtualW));
    vy = std::clamp(vy, 0.0f, static_cast<float>(virtualH));
    return { vx, vy };
}
