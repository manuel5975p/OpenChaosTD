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

// Recomputes the letterbox in framebuffer pixels and caches the logical->pixel
// mouse factor. We measure that factor as render/screen rather than trusting
// GetWindowScaleDPI(): on X11 fractional scaling the framebuffer often is not
// enlarged even though GetWindowScaleDPI() still reports e.g. 1.5x, which would
// over-scale the mouse. render/screen is 1.0 when unscaled, the real ratio when not.
void Screen::UpdateScale() {
    const int screenW = GetScreenWidth();
    m_dpiScale = (screenW > 0) ? static_cast<float>(GetRenderWidth()) / static_cast<float>(screenW)
                               : 1.0f;

    const Letterbox lb = ComputeLetterbox(GetRenderWidth(), GetRenderHeight(),
                                          m_virtualWidth, m_virtualHeight);
    m_destRect = lb.destRect;
    m_scale    = lb.scale;
}

// Frame
void Screen::BeginFrame() {
    BeginDrawing();
    ClearBackground(BLACK); // fills letterbox bars

    // Replace raylib's screen-space ortho with one spanning the window in
    // virtual units, placing the virtual rect at the letterboxed destination.
    // Game code keeps drawing in virtual coordinates; the GPU rasterizes at
    // native resolution. Modelview is untouched, so Camera2D nests as usual.
    const double renderW = static_cast<double>(GetRenderWidth());
    const double renderH = static_cast<double>(GetRenderHeight());
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    const double left = -m_destRect.x / m_scale;
    const double top  = -m_destRect.y / m_scale;
    rlOrtho(left, left + renderW / m_scale,
            top + renderH / m_scale, top, 0.0, 1.0);
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

// Scissor in virtual coordinates.
// m_destRect/m_scale are already in framebuffer pixels, so we drive rlScissor
// directly (GL's bottom-left origin) rather than BeginScissorMode, whose own
// DPI fix-up would double-scale our already-framebuffer-space rect.
void Screen::BeginScissor(Rectangle virtualRect) const {
    const int x = static_cast<int>(m_destRect.x + virtualRect.x * m_scale);
    const int y = static_cast<int>(m_destRect.y + virtualRect.y * m_scale);
    const int w = static_cast<int>(virtualRect.width  * m_scale);
    const int h = static_cast<int>(virtualRect.height * m_scale);

    rlDrawRenderBatchActive(); // flush draws bound to the previous scissor
    rlEnableScissorTest();
    rlScissor(x, GetRenderHeight() - (y + h), w, h);
}

void Screen::EndScissor() const {
    // Fall back to the letterbox clip rather than disabling clipping outright.
    BeginScissor({0.0f, 0.0f,
                  static_cast<float>(m_virtualWidth),
                  static_cast<float>(m_virtualHeight)});
}

// Virtual mouse
// GetMousePosition reports logical points; MapMouseToVirtual bridges those to
// framebuffer pixels via the DPI scale before undoing the letterbox.
Vector2 Screen::GetVirtualMouse() const {
    const Letterbox lb{ m_destRect, m_scale };
    return MapMouseToVirtual(GetMousePosition(), m_dpiScale, lb,
                             m_virtualWidth, m_virtualHeight);
}
