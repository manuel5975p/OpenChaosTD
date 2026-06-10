#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <world/desc_line.hpp> // DescLine (shared by tower + enemy modules)

// Read-only data structures handed to the HUD each frame by PlayingState. They carry a snapshot
// of gameplay state so the HUD never queries GameData / WaveManager / factories directly — the
// HUD only renders these views and raises HudSignals back.

// Top status bar.
struct StatusView {
    int  m_lives = 0;
    int  m_gold = 0;
    int  m_waveNumber = 0;
    int  m_victoryWave = 0;   // 0 = endless
    bool m_waveActive = false;
    bool m_autoSpawn = false;
    int  m_speed = 1;
};

// Bottom tower-build bar. Button names/textures/costs are static config captured at Build time;
// only the current gold changes per frame.
struct BuildBarView {
    int m_gold = 0;
};

// One enemy entry in the WaveHUD "Next Wave" panel (precomputed from a wave group + prototype).
struct WaveEnemyEntry {
    int m_count = 1;
    std::string m_name;
    std::string m_textureKey;
    int m_level = 1;
    bool m_hasProto = false;
    std::vector<DescLine> m_stats; // collected from the prototype's modules' DescribeStats
};

struct WaveView {
    float m_budget = 0.0f;
    std::vector<WaveEnemyEntry> m_entries;
};

// Tower inspection / hover-preview panel. Everything is precomputed by PlayingState so the HUD
// touches no Tower / Enemy / GameData type.
struct TowerInfoView {
    std::string m_name;
    std::string m_description;
    std::vector<DescLine> m_statLines;

    bool m_hasAttack = false;   // false = a wall (no combat UI)
    bool m_interactive = false; // a real selected tower (shows config buttons) vs a hover preview
    bool m_waveActive = false;  // sell shown but disabled mid-wave

    int m_sellRefund = 0;
    int m_level = 0;            // 0-based current level
    int m_upgradeCount = 0;     // number of upgrade tiers (0 = none)

    // Upgrade block (meaningful only when m_upgradeCount > 0)
    bool m_upgradeAtMax = false;
    int  m_upgradeCost = 0;
    bool m_upgradeReady = false; // affordable and not yet max level
    std::vector<DescLine> m_upgradePreview;

    std::string m_targetingName;

    // Placement: screen anchor the panel is positioned against, plus screen bounds for clamping.
    Vector2 m_screenPos = {0.0f, 0.0f};
    int m_screenW = 0;
    int m_screenH = 0;
};
