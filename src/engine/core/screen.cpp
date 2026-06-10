#include <engine/core/screen.hpp>

#include <rlgl.h>

// Init
void Screen::Init(int virtualWidth, int virtualHeight) {
    m_virtualWidth  = virtualWidth;
    m_virtualHeight = virtualHeight;

    UpdateScale();
}

// Resize
void Screen::OnResize() {
    UpdateScale();
}

// UpdateScale
// Calculates the largest rectangle that fits the virtual resolution inside the current screen
void Screen::UpdateScale() {
    const float screenW = static_cast<float>(GetScreenWidth());
    const float screenH = static_cast<float>(GetScreenHeight());

    const float scaleX = screenW / static_cast<float>(m_virtualWidth);
    const float scaleY = screenH / static_cast<float>(m_virtualHeight);
    m_scale = (scaleX < scaleY) ? scaleX : scaleY; // fit inside, preserve ratio

    const float scaledW = m_virtualWidth  * m_scale;
    const float scaledH = m_virtualHeight * m_scale;

    // Center the game image
    const float offsetX = (screenW - scaledW) * 0.5f;
    const float offsetY = (screenH - scaledH) * 0.5f;

    m_destRect = { offsetX, offsetY, scaledW, scaledH };
}

// Frame
void Screen::BeginFrame() {
    BeginDrawing();
    ClearBackground(BLACK); // fills letterbox bars

    // Replace raylib's screen-space ortho with one spanning the window in
    // virtual units, placing the virtual rect at the letterboxed destination.
    // Game code keeps drawing in virtual coordinates; the GPU rasterizes at
    // native resolution. Modelview is untouched, so Camera2D nests as usual.
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    const double left = -m_destRect.x / m_scale;
    const double top  = -m_destRect.y / m_scale;
    rlOrtho(left, left + GetScreenWidth() / m_scale,
            top + GetScreenHeight() / m_scale, top, 0.0, 1.0);
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();

    // Confine all game drawing (incl. ClearBackground) to the letterboxed area.
    BeginScissor({0.0f, 0.0f,
                  static_cast<float>(m_virtualWidth),
                  static_cast<float>(m_virtualHeight)});
}

void Screen::EndFrame() {
    EndScissorMode();
    EndDrawing();
}

// Scissor in virtual coordinates
void Screen::BeginScissor(Rectangle virtualRect) const {
    BeginScissorMode(static_cast<int>(m_destRect.x + virtualRect.x * m_scale),
                     static_cast<int>(m_destRect.y + virtualRect.y * m_scale),
                     static_cast<int>(virtualRect.width * m_scale),
                     static_cast<int>(virtualRect.height * m_scale));
}

void Screen::EndScissor() const {
    // Fall back to the letterbox clip rather than disabling clipping outright.
    BeginScissor({0.0f, 0.0f,
                  static_cast<float>(m_virtualWidth),
                  static_cast<float>(m_virtualHeight)});
}

// Virtual mouse
Vector2 Screen::GetVirtualMouse() const {
    Vector2 mouse = GetMousePosition();

    // Subtract letterbox offset, then divide by scale
    mouse.x = (mouse.x - m_destRect.x) / m_scale;
    mouse.y = (mouse.y - m_destRect.y) / m_scale;

    // Clamp to virtual bounds
    if (mouse.x < 0.0f) mouse.x = 0.0f;
    if (mouse.y < 0.0f) mouse.y = 0.0f;
    if (mouse.x > static_cast<float>(m_virtualWidth))  mouse.x = static_cast<float>(m_virtualWidth);
    if (mouse.y > static_cast<float>(m_virtualHeight)) mouse.y = static_cast<float>(m_virtualHeight);

    return mouse;
}
