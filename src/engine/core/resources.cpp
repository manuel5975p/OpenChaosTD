#include <engine/core/resources.hpp>
#include <raylib.h>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <unordered_set>


// Set Asset Path
void Resources::SetAssetPath(const std::string& assetPath){
    m_assetPath = assetPath;
}

// Path resolution
std::string Resources::ResolvePath(const std::string& relativePath) const {
    if (m_assetPath.empty())
        throw std::runtime_error("Resources: SetAssetPath() must be called before loading resources");

    return (std::filesystem::path(m_assetPath) / relativePath).string();
}

// Load
void Resources::LoadTexture(const std::string& key, const std::string& relativePath) {
    if (m_textures.count(key)) return;

    std::string fullPath = ResolvePath(relativePath);
    Texture2D tex = ::LoadTexture(fullPath.c_str());
    if (tex.id == 0)
        throw std::runtime_error("Resources: failed to load texture '" + fullPath + "'");

    m_textures[key] = tex;
}

void Resources::LoadTexturesFromDir(const std::string& relativeDir) {
    static const std::unordered_set<std::string> imageExts = { ".png", ".jpg", ".jpeg", ".bmp", ".tga" };

    std::filesystem::path dir = std::filesystem::path(m_assetPath) / relativeDir;
    if (!std::filesystem::is_directory(dir)) {
        std::cerr << "Resources: texture directory not found: " << dir << "\n";
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;
        if (!imageExts.count(entry.path().extension().string())) continue;

        std::string key = entry.path().stem().string();
        if (m_textures.count(key)) continue; // already loaded

        Texture2D tex = ::LoadTexture(entry.path().string().c_str());
        if (tex.id == 0) {
            std::cerr << "Resources: failed to load texture '" << entry.path() << "'\n";
            continue;
        }
        m_textures[key] = tex;
        std::cout << "Resources: loaded texture '" << key << "'\n";
    }
}

void Resources::LoadMusicFromDir(const std::string& relativeDir) {
    static const std::unordered_set<std::string> musicExts = { ".wav", ".ogg", ".mp3", ".flac", ".xm", ".mod" };

    std::filesystem::path dir = std::filesystem::path(m_assetPath) / relativeDir;
    if (!std::filesystem::is_directory(dir)) {
        std::cerr << "Resources: music directory not found: " << dir << "\n";
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;
        if (!musicExts.count(entry.path().extension().string())) continue;

        std::string key = entry.path().stem().string();
        if (m_music.count(key)) continue;

        Music music = ::LoadMusicStream(entry.path().string().c_str());
        if (music.frameCount == 0) {
            std::cerr << "Resources: failed to load music '" << entry.path() << "'\n";
            continue;
        }
        m_music[key] = music;
        std::cout << "Resources: loaded music '" << key << "'\n";
    }
}

void Resources::LoadSound(const std::string& key, const std::string& relativePath) {
    if (m_sounds.count(key)) return;

    std::string fullPath = ResolvePath(relativePath);
    Sound sfx = ::LoadSound(fullPath.c_str());
    if (sfx.frameCount == 0)
        throw std::runtime_error("Resources: failed to load sound '" + fullPath + "'");

    m_sounds[key] = sfx;
}

void Resources::LoadFont(const std::string& key, const std::string& relativePath, int fontSize) {
    if (m_fonts.count(key)) return;

    std::string fullPath = ResolvePath(relativePath);
    Font font = ::LoadFontEx(fullPath.c_str(), fontSize, nullptr, 0);
    if (font.texture.id == 0)
        throw std::runtime_error("Resources: failed to load font '" + fullPath + "'");

    m_fonts[key] = font;
}

void Resources::LoadMusic(const std::string& key, const std::string& relativePath) {
    if (m_music.count(key)) return;

    std::string fullPath = ResolvePath(relativePath);
    Music music = ::LoadMusicStream(fullPath.c_str());
    if (music.frameCount == 0)
        throw std::runtime_error("Resources: failed to load music '" + fullPath + "'");

    m_music[key] = music;
}

// Retrieve
Texture2D& Resources::GetTexture(const std::string& key) {
    auto it = m_textures.find(key);
    if (it == m_textures.end())
        throw std::runtime_error("Resources: texture key '" + key + "' not found");
    return it->second;
}

Sound& Resources::GetSound(const std::string& key) {
    auto it = m_sounds.find(key);
    if (it == m_sounds.end())
        throw std::runtime_error("Resources: sound key '" + key + "' not found");
    return it->second;
}

Font& Resources::GetFont(const std::string& key) {
    auto it = m_fonts.find(key);
    if (it == m_fonts.end())
        throw std::runtime_error("Resources: font key '" + key + "' not found");
    return it->second;
}

Music& Resources::GetMusic(const std::string& key) {
    auto it = m_music.find(key);
    if (it == m_music.end())
        throw std::runtime_error("Resources: music key '" + key + "' not found");
    return it->second;
}

//  Query
bool Resources::HasTexture(const std::string& key) const { return m_textures.count(key) > 0; }
bool Resources::HasSound(const std::string& key)   const { return m_sounds.count(key)   > 0; }
bool Resources::HasFont(const std::string& key)    const { return m_fonts.count(key)    > 0; }
bool Resources::HasMusic(const std::string& key)   const { return m_music.count(key)    > 0; }

// Shutdown
void Resources::Shutdown() {
    for (auto& [key, tex]  : m_textures) UnloadTexture(tex);
    for (auto& [key, sfx]  : m_sounds)   UnloadSound(sfx);
    for (auto& [key, font] : m_fonts)    UnloadFont(font);
    for (auto& [key, music]: m_music)    UnloadMusicStream(music);

    m_textures.clear();
    m_sounds.clear();
    m_fonts.clear();
    m_music.clear();
}