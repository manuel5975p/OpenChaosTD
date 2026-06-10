#pragma once

#include <raylib.h>

// Letterboxed virtual-resolution presenter: maps the fixed virtual coordinate
// space onto the window via the projection matrix, so everything rasterizes at
// native resolution (no intermediate render texture — vector text stays crisp).
class Screen {
public:
    // Call after InitWindow()
    void Init(int virtualWidth, int virtualHeight);

    // Call when the window is resized
    void OnResize();

    // Wraps all game drawing — call BeginFrame() / EndFrame() each loop
    void BeginFrame();
    void EndFrame();

    // Clips drawing to a rect in virtual coordinates (use instead of raylib's
    // BeginScissorMode inside a frame); EndScissor restores the letterbox clip.
    void BeginScissor(Rectangle virtualRect) const;
    void EndScissor() const;

    // Converts real screen mouse coords to virtual game coords
    Vector2 GetVirtualMouse() const;

    int GetGameWidth() const { return m_virtualWidth;  }
    int GetGameHeight() const { return m_virtualHeight; }

    float GetScale() const { return m_scale; }
    Rectangle GetDestRect() const { return m_destRect; }

private:
    // Recalculates the letterboxed destination rectangle and scale
    void UpdateScale();

    int m_virtualWidth = 1280;
    int m_virtualHeight = 720;

    Rectangle m_destRect = {};  // destination rect on the real screen (letterboxed)
    float m_scale = 1.0f;
};
