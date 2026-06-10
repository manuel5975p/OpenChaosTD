#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <engine/features/particle_system.hpp>
#include <engine/util/file_store.hpp>

// Loads all EmitterDesc presets from data/particle_effects.toml.
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

    // Inserts or replaces a preset in the runtime registry only (no file I/O).
    // Stored presets keep their address, so pointers from GetPtr stay valid.
    void Set(const std::string& name, const EmitterDesc& desc);

    // All preset names, sorted alphabetically (for UI listings).
    std::vector<std::string> Names() const;

    // Writes the preset into data/particle_effects.toml and upserts the registry.
    // The whole file is rewritten through toml++: comments are not preserved and
    // tables come out sorted by key. Returns false (and leaves the file
    // untouched) if the file exists but cannot be read.
    bool SavePreset(FileStore& fileStore, const std::string& name, const EmitterDesc& desc);

    // Removes the preset from data/particle_effects.toml and the registry.
    // Same full-rewrite semantics as SavePreset; pointers from GetPtr to the
    // removed preset are invalidated. Returns false if the file cannot be read.
    bool DeletePreset(FileStore& fileStore, const std::string& name);

private:
    std::unordered_map<std::string, EmitterDesc> m_presets;
};
