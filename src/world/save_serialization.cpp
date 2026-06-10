#include <world/save_serialization.hpp>

#include <world/map.hpp>
#include <factory/tower_factory.hpp>

bool LoadTowers(const nlohmann::json& j, DenseSlotMap<Tower>& out,
                const TowerFactory& factory, const Map& restoredMap) {
    if (!j.is_object()) return false;
    if (!j.contains("slots")    || !j["slots"].is_array())    return false;
    if (!j.contains("erase")    || !j["erase"].is_array())    return false;
    if (!j.contains("freeList") || !j["freeList"].is_array()) return false;
    if (!j.contains("values")   || !j["values"].is_array())   return false;

    const nlohmann::json& jslots  = j["slots"];
    const nlohmann::json& jvalues = j["values"];

    // Parse the sparse bookkeeping verbatim.
    std::vector<DenseSlotMap<Tower>::Slot> slots;
    slots.reserve(jslots.size());
    for (const auto& s : jslots) {
        DenseSlotMap<Tower>::Slot slot;
        slot.generation  = s.value("generation", 0u);
        slot.dense_index = s.value("dense", 0u);
        slot.occupied    = s.value("occupied", false);
        slots.push_back(slot);
    }
    std::vector<uint32_t> erase    = j["erase"].get<std::vector<uint32_t>>();
    std::vector<uint32_t> freeList = j["freeList"].get<std::vector<uint32_t>>();

    const size_t valueCount = jvalues.size();

    // Consistency checks — reject corrupt/incompatible saves before touching live state.
    if (erase.size() != valueCount) return false;

    size_t occupiedCount = 0;
    for (const auto& slot : slots)
        if (slot.occupied) ++occupiedCount;
    if (occupiedCount != valueCount) return false;

    for (size_t i = 0; i < valueCount; ++i) {
        uint32_t slotIdx = erase[i];
        if (slotIdx >= slots.size())           return false;
        if (!slots[slotIdx].occupied)          return false;
        if (slots[slotIdx].dense_index != i)   return false;
    }
    for (uint32_t f : freeList) {
        if (f >= slots.size())  return false;
        if (slots[f].occupied)  return false;
    }

    // Map dense index -> terrain modifier (occupied tiles only). The buff was baked into the
    // tower's base stats at placement (WorldSystem::PlaceTower) but never stored on the tower,
    // so it must be re-applied here before replaying upgrades to reproduce the exact stat math.
    std::vector<const TileModifier*> denseModifier(valueCount, nullptr);
    for (const Tile& tile : restoredMap.GetGrid()) {
        if (tile.m_towerKey == DenseSlotMap<Tower>::INVALID_KEY) continue;
        uint32_t slotIdx = tile.m_towerKey.index;
        if (slotIdx >= slots.size()) continue;
        const auto& slot = slots[slotIdx];
        if (!slot.occupied || slot.generation != tile.m_towerKey.generation) continue;
        if (slot.dense_index >= valueCount) continue;
        if (tile.m_modifier.Active())
            denseModifier[slot.dense_index] = &tile.m_modifier;
    }

    // Reconstruct each tower in dense order through the factory pipeline.
    std::vector<Tower> values;
    values.reserve(valueCount);
    for (size_t i = 0; i < valueCount; ++i) {
        const nlohmann::json& v = jvalues[i];
        std::string name = v.value("name", std::string{});
        if (!factory.Has(name)) return false; // unknown tower (datapack changed) -> abort

        Tower tw = factory.Create(name);

        // Terrain buff first (matches PlaceTower ordering), then upgrade tiers 0..level-1.
        if (const TileModifier* mod = denseModifier[i])
            tw.PatchStats(mod->m_statKey, mod->m_value, mod->m_mul);

        int level = v.value("level", 0);
        if (tw.m_upgrades) {
            int maxLevel = static_cast<int>(tw.m_upgrades->size());
            for (int l = 0; l < level && l < maxLevel; ++l)
                factory.ApplyUpgradeStats(tw, (*tw.m_upgrades)[l]);
        }

        if (v.contains("position")) tw.m_position = v["position"].get<Vector2>();
        tw.m_cooldown = v.value("cooldown", 0.0f);
        tw.m_cost     = v.value("cost", tw.m_cost);
        tw.m_level    = level;

        values.push_back(std::move(tw));
    }

    out.RawAssign(std::move(slots), std::move(values), std::move(erase), std::move(freeList));
    return true;
}
