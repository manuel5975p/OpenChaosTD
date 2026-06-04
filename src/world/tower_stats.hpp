#pragma once

#include <string>
#include <vector>
#include <cstdio>
#include <raylib.h>

// One described line: a formatted text row plus the color it should render in.
// Shared by the multi-line Describe() of TowerStats and TowerUpgrade (single-line
// TowerModule::Describe still writes a lone string/color pair directly).
struct DescLine {
    std::string m_text;
    Color m_color;
};

enum class TargetingMode {
    First,
    Last,
    MostHealth,
    LowestHealth,
    Fastest,
    Slowest,
    MostArmor,
    MostShield
};

inline constexpr int kTargetingModeCount = static_cast<int>(TargetingMode::MostShield) + 1; // MostShield stays last

inline TargetingMode NextTargetingMode(TargetingMode m) {
    return static_cast<TargetingMode>((static_cast<int>(m) + 1) % kTargetingModeCount);
}

inline const char* TargetingModeName(TargetingMode m) {
    switch (m) {
        case TargetingMode::First:          return "First";
        case TargetingMode::Last:           return "Last";
        case TargetingMode::MostHealth:     return "Most HP";
        case TargetingMode::LowestHealth:   return "Least HP";
        case TargetingMode::Fastest:        return "Fastest";
        case TargetingMode::Slowest:        return "Slowest";
        case TargetingMode::MostArmor:      return "Most Armor";
        case TargetingMode::MostShield:     return "Most Shield";
    }
    return "First";
}

struct TowerStats {
    float m_damage = 0.0f;
    float m_shotsPerMinute = 0.0f; // cooldown between shots = 60 / m_shotsPerMinute
    float m_range = 0.0f;
    int m_targetCount = 0;
    TargetingMode m_targetingMode = TargetingMode::First;
    float m_armorPierce = 0.0f;

    // Append the core combat stats as display lines, mirroring TowerModule::Describe.
    // All fields here are flat values, so they render as plain numbers (no percentages).
    void Describe(std::vector<DescLine>& out) const;
};

inline void TowerStats::Describe(std::vector<DescLine>& out) const {
    char buf[40];
    snprintf(buf, sizeof(buf), "Damage:  %g", m_damage);
    out.push_back({buf, RAYWHITE});
    snprintf(buf, sizeof(buf), "Range:   %.0f", m_range);
    out.push_back({buf, RAYWHITE});
    snprintf(buf, sizeof(buf), "Rate:    %d/min", static_cast<int>(m_shotsPerMinute + 0.5f));
    out.push_back({buf, RAYWHITE});
    snprintf(buf, sizeof(buf), "Targets: %d", m_targetCount);
    out.push_back({buf, RAYWHITE});
    // Pierce only shows when present, highlighted like the old HUD did.
    if (m_armorPierce > 0.0f) {
        snprintf(buf, sizeof(buf), "Pierce:  %g", m_armorPierce);
        out.push_back({buf, GOLD});
    }
}

// Apply an additive (mul=false) or multiplicative (mul=true) delta to one numeric field.
// Shared by ApplyStat and every TowerModule::PatchStat.
inline void ApplyDelta(float& field, float v, bool mul) {
    field = mul ? field * v : field + v;
}

// Apply an upgrade delta to a named stat field (additive, or multiplicative when mul=true).
inline void ApplyStat(TowerStats& s, const std::string& key, float v, bool mul) {
    if      (key == "damage")         ApplyDelta(s.m_damage, v, mul);
    else if (key == "shotsPerMinute") ApplyDelta(s.m_shotsPerMinute, v, mul);
    else if (key == "range")          ApplyDelta(s.m_range, v, mul);
    else if (key == "armorPierce")    ApplyDelta(s.m_armorPierce, v, mul);
    else if (key == "targetCount") {
        float t = static_cast<float>(s.m_targetCount);
        ApplyDelta(t, v, mul);
        s.m_targetCount = static_cast<int>(t + 0.5f);
    }
}
