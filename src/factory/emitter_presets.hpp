#pragma once

#include <string>
#include <unordered_map>
#include <engine/features/particle_system.hpp>
#include <engine/util/file_store.hpp>

// Loads all EmitterDesc presets from data/particle_effects.json.
// Passed by const-ref to any factory that needs to resolve preset names.
class EmitterPresets {
public:
    void Load(FileStore& fileStore);

    // Returns the named preset by value. Logs and returns a count=0 default if not found.
    EmitterDesc Get(const std::string& name) const;

    // Returns a stable pointer to the stored preset, valid for the lifetime of EmitterPresets.
    // Returns nullptr and logs if the name is unknown.
    const EmitterDesc* GetPtr(const std::string& name) const;

    bool Has(const std::string& name) const;

private:
    std::unordered_map<std::string, EmitterDesc> m_presets;
};
