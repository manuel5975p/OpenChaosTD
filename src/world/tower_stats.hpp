#pragma once

#include <string>

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
};

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
