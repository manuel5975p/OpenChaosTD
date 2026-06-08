#include <factory/emitter_presets.hpp>
#include <toml++/toml.hpp>
#include <iostream>

static Color ParseColor(const toml::array& a) {
    return {
        (unsigned char)a[0].value_or(0), (unsigned char)a[1].value_or(0),
        (unsigned char)a[2].value_or(0), (unsigned char)a[3].value_or(0)
    };
}

static SpawnShape ParseShape(const std::string& s) {
    if (s == "Line")   return SpawnShape::Line;
    if (s == "Box")    return SpawnShape::Box;
    if (s == "Circle") return SpawnShape::Circle;
    if (s == "Ring")   return SpawnShape::Ring;
    return SpawnShape::Point;
}

static EmitterDesc ParseEmitterDesc(const toml::table& j) {
    EmitterDesc d;
    if (auto c = j["color"].as_array())    d.m_color    = ParseColor(*c);
    if (auto c = j["endColor"].as_array()) d.m_endColor = ParseColor(*c);
    d.m_count         = j["count"].value_or(0);
    d.m_speed         = j["speed"].value_or(50.0f);
    d.m_speedVariance = j["speedVariance"].value_or(20.0f);
    d.m_spread        = j["spread"].value_or(360.0f);
    d.m_angle         = j["angle"].value_or(0.0f);
    d.m_lifetime      = j["lifetime"].value_or(0.2f);
    d.m_size          = j["size"].value_or(3.0f);
    d.m_endSize       = j["endSize"].value_or(0.0f);

    // Spawn shape, unified dynamics, and continuous emission rate.
    d.m_shape           = ParseShape(j["shape"].value_or(std::string("Point")));
    d.m_shapeSize       = {j["shapeWidth"].value_or(0.0f), j["shapeHeight"].value_or(0.0f)};
    d.m_shapeRadius     = j["shapeRadius"].value_or(0.0f);
    d.m_radialSpeed     = j["radialSpeed"].value_or(0.0f);
    d.m_tangentialSpeed = j["tangentialSpeed"].value_or(0.0f);
    d.m_emitRate        = j["emitRate"].value_or(0.0f);

    return d;
}

void EmitterPresets::Load(FileStore& fileStore) {
    auto data = fileStore.LoadToml("data/particle_effects.toml");
    auto presets = data["presets"].as_table();
    if (!presets) {
        std::cerr << "EmitterPresets: failed to load particle_effects.toml\n";
        return;
    }
    for (auto&& [name, descNode] : *presets) {
        if (auto descTbl = descNode.as_table()) {
            std::string nm(name.str());
            m_presets[nm] = ParseEmitterDesc(*descTbl);
            std::cout << "EmitterPresets: loaded '" << nm << "'\n";
        }
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
