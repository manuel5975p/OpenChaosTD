#pragma once

#include <string>
#include <vector>
#include <world/desc_line.hpp>

// Shared display/upgrade interface for tower and enemy modules. Both hierarchies append
// info-panel rows and accept upgrade patches through these two hooks with identical
// signatures; the behavioral hooks (firing, cloning, damage interception, …) stay on the
// concrete TowerModule / EnemyModule bases.
class IStatModule {
public:
    virtual ~IStatModule() = default;
    // Append this module's display lines to the info panel (zero or more rows).
    virtual void DescribeStats(std::vector<DescLine>&) const {}
    // Patch a module-owned parameter from the upgrade system (e.g. slowPercent, armor).
    virtual void PatchStats(const std::string& /*key*/, float /*v*/, bool /*mul*/) {}
};
