#pragma once

#include <nlohmann/json.hpp>
#include <toml++/toml.hpp>
#include <string>

class FileStore {
public:
    FileStore()  = default;
    ~FileStore() = default;

    // Non-copyable — owns internal root path state
    FileStore(const FileStore&)            = delete;
    FileStore& operator=(const FileStore&) = delete;

    // Call once, project root as its parent for files stored.
    void SetRootPath(const std::string& assetPath);
    
    const std::string& GetRootPath() const { return m_rootPath; }

    // JSON file I/O (nlohmann::json)
    void           SaveJson(const std::string& path, const nlohmann::json& data);
    nlohmann::json LoadJson(const std::string& path);

    // TOML file I/O (toml++)
    void           SaveToml(const std::string& path, const toml::table& data);
    toml::table    LoadToml(const std::string& path);

    // Raw text I/O — byte-exact, no parsing. For content the structured savers
    // can't round-trip (e.g. TOML files whose comments must survive a rewrite).
    void           SaveText(const std::string& path, const std::string& text);
    std::string    LoadText(const std::string& path);

    // Format-agnostic (operate on the path, regardless of file contents)
    bool           Exists(const std::string& path);
    void           Delete(const std::string& path);

private:
    // Resolves a relative path to a full absolute path (native only)
    std::string ResolvePath(const std::string& path) const;

    std::string m_rootPath; // Set by Init() — absolute path to project root
};