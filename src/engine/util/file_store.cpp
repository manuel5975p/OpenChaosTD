#include <engine/util/file_store.hpp>
#include <iostream>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
    #include <fstream>
    #include <sstream>
#else
    #include <fstream>
    #include <filesystem>
#endif

// RootPath
void FileStore::SetRootPath(const std::string& rootPath) {
#if defined(PLATFORM_WEB)
    m_rootPath = rootPath;
    std::cout << "filestore: root set to '" << m_rootPath << "' (web)\n";
#else
    std::filesystem::path root = std::filesystem::path(rootPath);
    m_rootPath = root.string();
    std::cout << "filestore: root set to '" << m_rootPath << "'\n";
#endif
}

// Private: path resolution
std::string FileStore::ResolvePath(const std::string& path) const {
    if (m_rootPath.empty())
        throw std::runtime_error("filestore: SetRootPath() must be called before any file operations");

#if defined(PLATFORM_WEB)
    return path;
#else
    return (std::filesystem::path(m_rootPath) / path).string();
#endif
}

// SaveJson
void FileStore::SaveJson(const std::string& path, const nlohmann::json& data) {
#if defined(PLATFORM_WEB)

    std::string jsonStr = data.dump();
    EM_ASM({
        localStorage.setItem(UTF8ToString($0), UTF8ToString($1));
    }, path.c_str(), jsonStr.c_str());
    std::cout << "filestore: saved '" << path << "' to localStorage\n";

#else

    std::string fullPath = ResolvePath(path);
    std::filesystem::create_directories(
        std::filesystem::path(fullPath).parent_path()
    );

    std::ofstream file(fullPath);
    if (!file.is_open()) {
        std::cerr << "filestore: could not write '" << fullPath << "'\n";
        return;
    }
    file << data.dump(4) << "\n";
    std::cout << "filestore: saved '" << fullPath << "'\n";

#endif
}

// LoadJson
nlohmann::json FileStore::LoadJson(const std::string& path) {
#if defined(PLATFORM_WEB)

    // Try VFS first (preloaded read-only game data)
    {
        std::ifstream vfsFile(path);
        if (vfsFile.is_open()) {
            try {
                nlohmann::json data;
                vfsFile >> data;
                return data;
            } catch (const nlohmann::json::parse_error& e) {
                std::cerr << "filestore: parse error loading '" << path << "': " << e.what() << "\n";
                return {};
            }
        }
    }

    // Fall back to localStorage (runtime save data)
    char* raw = (char*)EM_ASM_PTR({
        var value = localStorage.getItem(UTF8ToString($0));
        if (!value) return 0;
        var len = lengthBytesUTF8(value) + 1;
        var buf = _malloc(len);
        stringToUTF8(value, buf, len);
        return buf;
    }, path.c_str());

    if (!raw) {
        std::cout << "filestore: no data found for '" << path << "'\n";
        return {};
    }

    std::string jsonStr(raw);
    free(raw);

    try {
        return nlohmann::json::parse(jsonStr);
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "filestore: parse error loading '" << path << "': " << e.what() << "\n";
        return {};
    }

#else

    std::string fullPath = ResolvePath(path);
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        std::cout << "filestore: no file found at '" << fullPath << "'\n";
        return {};
    }

    try {
        nlohmann::json data;
        file >> data;
        return data;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "filestore: parse error loading '" << fullPath << "': " << e.what() << "\n";
        return {};
    }

#endif
}

// SaveToml
void FileStore::SaveToml(const std::string& path, const toml::table& data) {
#if defined(PLATFORM_WEB)

    std::stringstream ss;
    ss << data;
    std::string tomlStr = ss.str();
    EM_ASM({
        localStorage.setItem(UTF8ToString($0), UTF8ToString($1));
    }, path.c_str(), tomlStr.c_str());
    std::cout << "filestore: saved '" << path << "' to localStorage\n";

#else

    std::string fullPath = ResolvePath(path);
    std::filesystem::create_directories(
        std::filesystem::path(fullPath).parent_path()
    );

    std::ofstream file(fullPath);
    if (!file.is_open()) {
        std::cerr << "filestore: could not write '" << fullPath << "'\n";
        return;
    }
    file << data << "\n";
    std::cout << "filestore: saved '" << fullPath << "'\n";

#endif
}

// LoadToml
toml::table FileStore::LoadToml(const std::string& path) {
#if defined(PLATFORM_WEB)

    // Try VFS first (preloaded read-only game data)
    {
        std::ifstream vfsFile(path);
        if (vfsFile.is_open()) {
            try {
                return toml::parse(vfsFile);
            } catch (const toml::parse_error& e) {
                std::cerr << "filestore: parse error loading '" << path << "': " << e.description() << "\n";
                return {};
            }
        }
    }

    // Fall back to localStorage (runtime save data)
    char* raw = (char*)EM_ASM_PTR({
        var value = localStorage.getItem(UTF8ToString($0));
        if (!value) return 0;
        var len = lengthBytesUTF8(value) + 1;
        var buf = _malloc(len);
        stringToUTF8(value, buf, len);
        return buf;
    }, path.c_str());

    if (!raw) {
        std::cout << "filestore: no data found for '" << path << "'\n";
        return {};
    }

    std::string tomlStr(raw);
    free(raw);

    try {
        return toml::parse(tomlStr);
    } catch (const toml::parse_error& e) {
        std::cerr << "filestore: parse error loading '" << path << "': " << e.description() << "\n";
        return {};
    }

#else

    std::string fullPath = ResolvePath(path);
    if (!std::filesystem::exists(fullPath)) {
        std::cout << "filestore: no file found at '" << fullPath << "'\n";
        return {};
    }

    try {
        return toml::parse_file(fullPath);
    } catch (const toml::parse_error& e) {
        std::cerr << "filestore: parse error loading '" << fullPath << "': " << e.description() << "\n";
        return {};
    }

#endif
}

// Exists
bool FileStore::Exists(const std::string& path) {
#if defined(PLATFORM_WEB)

    // Check VFS first
    if (FILE* f = fopen(path.c_str(), "r")) {
        fclose(f);
        return true;
    }

    // Check localStorage
    int exists = EM_ASM_INT({
        return localStorage.getItem(UTF8ToString($0)) !== null ? 1 : 0;
    }, path.c_str());
    return exists == 1;

#else

    return std::filesystem::exists(ResolvePath(path));

#endif
}

// Delete
void FileStore::Delete(const std::string& path) {
#if defined(PLATFORM_WEB)

    EM_ASM({
        localStorage.removeItem(UTF8ToString($0));
    }, path.c_str());

#else

    std::string fullPath = ResolvePath(path);
    if (std::filesystem::exists(fullPath))
        std::filesystem::remove(fullPath);

#endif
}