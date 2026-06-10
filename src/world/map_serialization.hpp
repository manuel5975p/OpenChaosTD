#pragma once

#include <toml++/toml.hpp>
#include <string>

class Map;
class FileStore;

// Reads and writes a custom map as "<mapDir>/map.toml". Kept out of the Map class
// (which is included almost everywhere) so toml++ stays a serialization-only
// dependency — mirroring how EmitterPresets keeps toml off of EmitterDesc.
namespace MapSerialization {

// Authoring metadata stored alongside the grid; not part of Map itself.
struct MapMeta {
    std::string m_name;
    std::string m_description;
};

// Build the full TOML table for a map + metadata (inverse of Load).
toml::table BuildMapTable(const Map& map, const MapMeta& meta);

// Write "<mapDir>/map.toml". Returns false if the FileStore write fails.
bool Save(FileStore& fileStore, const std::string& mapDir, const Map& map, const MapMeta& meta);

// Read "<mapDir>/map.toml" and reconstruct outMap (grid + geometry + path mesh) and
// outMeta. Returns false on a missing/malformed file.
bool Load(FileStore& fileStore, const std::string& mapDir, Map& outMap, MapMeta& outMeta);

} // namespace MapSerialization
