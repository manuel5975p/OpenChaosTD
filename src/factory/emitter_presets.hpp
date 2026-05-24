#pragma once

#include <string>
#include <unordered_map>
#include <core/particle_system.hpp>
#include <core/jsonio.hpp>

// Loads all EmitterDesc presets from data/particle_effects.json.
// Passed by const-ref to any factory that needs to resolve preset names.
class EmitterPresets {
public:
    void Load(JsonIO& jsonio);

    // Returns the named preset by value. Logs and returns a count=0 default if not found.
    EmitterDesc Get(const std::string& name) const;

    // Returns a stable pointer to the stored preset, valid for the lifetime of EmitterPresets.
    // Returns nullptr and logs if the name is unknown.
    const EmitterDesc* GetPtr(const std::string& name) const;

    bool Has(const std::string& name) const;

private:
    std::unordered_map<std::string, EmitterDesc> m_presets;
};
