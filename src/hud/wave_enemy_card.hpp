#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <hud/hud_views.hpp>       // WaveEnemyEntry
#include <world/desc_line.hpp> // DescLine (shared by tower + enemy modules)

class Resources;

// One enemy entry in the WaveHUD "Next Wave" panel: a sprite tile on the left, a header row
// (count + name on the left, level badge right-aligned), then one stat row per enemy module.
// Content comes from a read-only WaveEnemyEntry, so the card never touches an Enemy directly.
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
    std::vector<DescLine> m_stats;

    // Pull count/name/level/stats from a precomputed wave entry.
    void SetContent(const WaveEnemyEntry& entry);

    // Height the card needs: pad + max(icon tile, header row + stat rows) + pad.
    float Measure() const;

    // Draw the frame, icon backing + sprite, header row, and stat rows within `bounds`.
    void Draw(Rectangle bounds, Resources& assets) const;
};
