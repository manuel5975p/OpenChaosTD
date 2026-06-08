#include <factory/emitter_presets.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

static Color ParseColor(const json& j) {
    return {
        (unsigned char)j[0].get<int>(), (unsigned char)j[1].get<int>(),
        (unsigned char)j[2].get<int>(), (unsigned char)j[3].get<int>()
    };
}

static EmitterDesc ParseEmitterDesc(const json& j) {
    EmitterDesc d;
    if (j.contains("color"))    d.m_color    = ParseColor(j["color"]);
    if (j.contains("endColor")) d.m_endColor = ParseColor(j["endColor"]);
    d.m_count         = j.value("count", 0);
    d.m_speed         = j.value("speed", 50.0f);
    d.m_speedVariance = j.value("speedVariance", 20.0f);
    d.m_spread        = j.value("spread", 360.0f);
    d.m_angle         = j.value("angle", 0.0f);
    d.m_lifetime      = j.value("lifetime", 0.2f);
    d.m_size          = j.value("size", 3.0f);
    d.m_endSize       = j.value("endSize", 0.0f);

    return d;
}

void EmitterPresets::Load(FileStore& fileStore) {
    auto data = fileStore.LoadJson("data/particle_effects.json");
    if (data.is_null() || !data.contains("presets")) {
        std::cerr << "EmitterPresets: failed to load particle_effects.json\n";
        return;
    }
    for (auto& [name, desc] : data["presets"].items()) {
        m_presets[name] = ParseEmitterDesc(desc);
        std::cout << "EmitterPresets: loaded '" << name << "'\n";
    }
}

EmitterDesc EmitterPresets::Get(const std::string& name) const {
    auto it = m_presets.find(name);
    if (it != m_presets.end()) return it->second;
    std::cerr << "EmitterPresets: unknown preset '" << name << "'\n";
    return {};
}

const EmitterDesc* EmitterPresets::GetPtr(const std::string& name) const {
    auto it = m_presets.find(name);
    if (it != m_presets.end()) return &it->second;
    std::cerr << "EmitterPresets: unknown preset '" << name << "'\n";
    return nullptr;
}

bool EmitterPresets::Has(const std::string& name) const {
    return m_presets.count(name) > 0;
}
