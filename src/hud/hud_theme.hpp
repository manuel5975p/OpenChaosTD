#pragma once

#include <raylib.h>

// Shared HUD theme: the single source of truth for panel colors, the sibling-layout anchor
// (status-bar height), and the per-panel layout metrics that several HUDs would otherwise each
// re-declare and re-scale. Keeping these here means a palette or DPI change touches one file.
namespace Hud {

// --- Palette ----------------------------------------------------------------
// Panel chrome.
inline constexpr Color kPanelBorder  {80, 80, 80, 255};
// Secondary / dim body text (descriptions, budgets, placeholders).
inline constexpr Color kTextMuted    {180, 180, 180, 255};
// Upgrade button label when the purchase is affordable.
inline constexpr Color kUpgradeReady {160, 240, 120, 255};
// Upgrade tooltip border (gold accent).
inline constexpr Color kTooltipBorder{255, 180, 0, 255};
// Wave enemy card frame.
inline constexpr Color kCardFill     {40, 40, 48, 200};
inline constexpr Color kCardBorder   {90, 90, 100, 255};
// Status-bar endless-mode infinity glyph.
inline constexpr Color kInfinityGlyph{120, 180, 220, 255};

// The dark panel fill is shared but drawn at varying opacity, so it is a helper rather than a
// constant. The event toast text likewise fades, so its RGB lives behind a helper too.
inline Color PanelBg(unsigned char alpha) { return {20, 20, 20, alpha}; }
inline Color EventText(unsigned char alpha) { return {255, 220, 80, alpha}; }

// --- Sibling layout anchor --------------------------------------------------
// Unscaled height of the top StatusHUD bar. EventHUD and WaveHUD position their content below the
// bar; reading this (then applying their own Scaled()) keeps them in sync if the bar height changes.
inline constexpr float kStatusBarBaseHeight = 36.0f;
// Extra gap WaveHUD leaves below the status bar.
inline constexpr float kWavePanelGap = 6.0f;

// --- Shared panel metrics ---------------------------------------------------
// The common set of scaled layout values every text panel needs. Panels compose this instead of
// re-declaring the same members; panels with extra metrics keep those as their own fields.
struct PanelMetrics {
    float panelW = 0.0f;
    float margin = 0.0f;
    float lineH = 0.0f;
    float headerH = 0.0f;
    int   fontSm = 0;
    int   fontHeader = 0;

    // Scale a set of unscaled base values once. The caller supplies the bases so panels with
    // different baselines (e.g. lineH 14 vs 15) still share one type.
    static PanelMetrics Scale(float scale, float baseW, float baseMargin, float baseLineH,
                              float baseHeaderH, float baseFontSm, float baseFontHeader) {
        PanelMetrics m;
        m.panelW = baseW * scale;
        m.margin = baseMargin * scale;
        m.lineH = baseLineH * scale;
        m.headerH = baseHeaderH * scale;
        m.fontSm = static_cast<int>(baseFontSm * scale);
        m.fontHeader = static_cast<int>(baseFontHeader * scale);
        return m;
    }
};

} // namespace Hud
