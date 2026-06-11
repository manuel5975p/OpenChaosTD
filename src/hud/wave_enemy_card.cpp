#include <hud/wave_enemy_card.hpp>
#include <engine/core/text.hpp>
#include <engine/core/draw_helpers.hpp>
#include <engine/core/resources.hpp>
#include <raylib.h>
#include <algorithm>

// Resolve a texture by key and aspect-fit it into `dest`. No-op if the key is missing.
static void DrawSpriteFit(const std::string& textureKey, Resources& assets, Rectangle dest) {
    if (!assets.HasTexture(textureKey)) return; // GetTexture throws on a missing key
    DrawTextureFitted(assets.GetTexture(textureKey), dest);
}

void WaveEnemyCard::SetContent(const WaveEnemyEntry& entry) {
    m_count      = entry.m_count;
    m_name       = entry.m_name;
    m_level      = entry.m_level;
    m_hasProto   = entry.m_hasProto;
    m_textureKey = entry.m_textureKey;
    m_stats      = entry.m_stats;
}

float WaveEnemyCard::Measure() const {
    // Header row plus one row per module stat; never shorter than the sprite tile.
    float textH = static_cast<float>(1 + m_stats.size()) * m_lineH;
    float contentH = std::max(m_iconSize, textH);
    return contentH + 2.0f * m_pad;
}

void WaveEnemyCard::Draw(Rectangle bounds, Resources& assets) const {
    // Card frame: subtle fill plus a border so each enemy type reads as a distinct tile.
    DrawRectangleRec(bounds, {40, 40, 48, 200});
    DrawRectangleLinesEx(bounds, 1.0f, {90, 90, 100, 255});

    // Icon tile on the left; the sprite draws directly over the card with a transparent backing.
    Rectangle icon = { bounds.x + m_pad, bounds.y + m_pad, m_iconSize, m_iconSize };
    if (m_hasProto)
        DrawSpriteFit(m_textureKey, assets, icon);

    // Text column to the right of the icon.
    float tx = icon.x + m_iconSize + m_pad;
    float ty = bounds.y + m_pad;
    float textRight = bounds.x + bounds.width - m_pad;

    // Header row: "Nx Name" on the left, "Lv N" badge right-aligned.
    Text::Draw(TextFormat("%dx %s", m_count, m_name.c_str()),
             static_cast<int>(tx), static_cast<int>(ty), m_fontSm, RAYWHITE);
    if (m_hasProto) {
        const char* lvlText = TextFormat("Lv %d", m_level);
        int lw = Text::Measure(lvlText, m_fontSm);
        Text::Draw(lvlText, static_cast<int>(textRight) - lw, static_cast<int>(ty), m_fontSm, GOLD);
    }
    ty += m_lineH;

    // One stat row per module, colored by the module (RAYWHITE core, Shield/Immune accents).
    for (const auto& line : m_stats) {
        Text::Draw(line.m_text.c_str(), static_cast<int>(tx), static_cast<int>(ty), m_fontSm, line.m_color);
        ty += m_lineH;
    }
}
