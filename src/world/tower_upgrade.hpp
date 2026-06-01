#pragma once

#include <string>
#include <vector>
#include <utility>
#include <nlohmann/json.hpp>

// One purchasable upgrade level for a tower: scalar deltas applied to TowerStats
// (additive + multiplicative) plus any new effect modules to append.
struct TowerUpgrade {
    int cost = 0;
    std::vector<std::pair<std::string, float>> adds; // stat key -> additive delta
    std::vector<std::pair<std::string, float>> muls; // stat key -> multiplicative factor
    std::vector<nlohmann::json> addModules;          // new effect modules built via TowerFactory
};
