#pragma once

#include <string>
#include <toml++/toml.hpp>

// A parsed module definition from a datapack: the raw TOML sub-tree plus its cached "type" string.
// Wrapping the table lets the world-layer upgrade structs reference module defs (e.g. to display
// "Adds Burn") without depending on toml++ — only the factories that build modules touch m_table.
struct ModuleDef {
    std::string m_type;   // value of the def's "type" key (cached for display without re-parsing)
    toml::table m_table;  // full module sub-tree, consumed by the factory's BuildModule
};

// Wrap a parsed module sub-tree with its cached "type" string for the world layer.
inline ModuleDef MakeModuleDef(const toml::table& t) {
    return ModuleDef{t["type"].value_or(std::string{}), t};
}
