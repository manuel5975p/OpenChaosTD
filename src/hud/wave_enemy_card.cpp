#include <hud/wave_enemy_card.hpp>
#include <systems/render_system.hpp>
#include <world/enemy.hpp>
#include <world/enemy_modules.hpp>
#include <raylib.h>
#include <algorithm>

void WaveEnemyCard::SetContent(int count, const std::string& name, const Enemy* proto) {
    m_count = count;
    m_name = name;
    m_stats.clear();
    m_hasProto = proto != nullptr;
    if (!proto) return;

    m_level = proto->m_level;
    m_textureKey = proto->m_presentation.m_texture;
    // Every module appends its rows (Health/Speed, Armor, Regen, Shield, Split, Immune, ...),
    // mirroring the tower info panel's per-module DescribeStats rendering.
    for (const auto& mod : proto->m_modules)
        mod->DescribeStats(m_stats);
}

float WaveEnemyCard::Measure() const {
    // Header row plus one row per module stat; never shorter than the sprite tile.
    float textH = static_cast<float>(1 + m_stats.size()) * m_lineH;
    float contentH = std::max(m_iconSize, textH);
    return contentH + 2.0f * m_pad;
}

void WaveEnemyCard::Draw(Rectangle bounds, const RenderSystem& render, Resources& assets) const {
    // Card frame: subtle fill plus a border so each enemy type reads as a distinct tile.
    DrawRectangleRec(bounds, {40, 40, 48, 200});
    DrawRectangleLinesEx(bounds, 1.0f, {90, 90, 100, 255});

    // Icon tile on the left; the sprite draws directly over the card with a transparent backing.
    Rectangle icon = { bounds.x + m_pad, bounds.y + m_pad, m_iconSize, m_iconSize };
    if (m_hasProto)
        render.DrawEnemyIcon(m_textureKey, assets, icon);

    // Text column to the right of the icon.
    float tx = icon.x + m_iconSize + m_pad;
    float ty = bounds.y + m_pad;
    float textRight = bounds.x + bounds.width - m_pad;

    // Header row: "Nx Name" on the left, "Lv N" badge right-aligned.
    DrawText(TextFormat("%dx %s", m_count, m_name.c_str()),
             static_cast<int>(tx), static_cast<int>(ty), m_fontSm, RAYWHITE);
    if (m_hasProto) {
        const char* lvlText = TextFormat("Lv %d", m_level);
        int lw = MeasureText(lvlText, m_fontSm);
        DrawText(lvlText, static_cast<int>(textRight) - lw, static_cast<int>(ty), m_fontSm, GOLD);
    }
    ty += m_lineH;

    // One stat row per module, colored by the module (RAYWHITE core, Shield/Immune accents).
    for (const auto& line : m_stats) {
        DrawText(line.m_text.c_str(), static_cast<int>(tx), static_cast<int>(ty), m_fontSm, line.m_color);
        ty += m_lineH;
    }
}
