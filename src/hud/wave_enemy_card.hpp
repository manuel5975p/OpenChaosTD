#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <world/tower_modules.hpp> // DescLine (shared by tower + enemy modules)

class Enemy;
class RenderSystem;
class Resources;

// One enemy entry in the WaveHUD "Next Wave" panel: a sprite tile on the left, a header row
// (count + name on the left, level badge right-aligned), then one stat row per enemy module.
// The stat rows are fed by each module's DescribeStats, exactly like the tower info panel, so the
// two panels share the same module-based layout and colors.
struct WaveEnemyCard {
    // Layout metrics (already scaled); seeded by WaveHUD from its own constants.
    float m_pad = 6.0f;      // inner padding of the card
    float m_iconSize = 44.0f; // square sprite tile on the left
    float m_lineH = 14.0f;   // row height for the header and each stat line
    int   m_fontSm = 11;

    // Content, filled by SetContent.
    int m_count = 1;
    std::string m_name;
    int m_level = 1;
    bool m_hasProto = false;
    std::string m_textureKey;
    std::vector<DescLine> m_stats; // collected from proto->m_modules' DescribeStats

    // Pull count/name/level/stats from a wave group + its (possibly null) preview prototype.
    void SetContent(int count, const std::string& name, const Enemy* proto);

    // Height the card needs: pad + max(icon tile, header row + stat rows) + pad.
    float Measure() const;

    // Draw the frame, icon backing + sprite, header row, and stat rows within `bounds`.
    void Draw(Rectangle bounds, const RenderSystem& render, Resources& assets) const;
};
