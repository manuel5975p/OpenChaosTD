#include <game.hpp>
#include <raylib.h>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <states/menu_state.hpp>

// Constructor / Destructor
Game::Game() {
    m_fileStore.SetRootPath(SearchFolderParentPath("resources", 5).parent_path());
    m_gameConfig.Load(m_fileStore);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(m_gameConfig.gameWidth, m_gameConfig.gameHeight, m_gameConfig.title.c_str());
    m_gameConfig.ApplyIcon();
    SetTargetFPS(m_gameConfig.fps);
    SetExitKey(KEY_NULL);
    InitAudioDevice();

    m_screen.Init(m_gameConfig.gameWidth, m_gameConfig.gameHeight);

    // Gameplay data and assets are no longer loaded here — they belong to a
    // datapack and load only once the player selects one (see ActivateDatapack).
    LoadResources();
    m_soundSystem.SetMusicVolume(m_gameConfig.musicVolume);
    m_soundSystem.SetSfxVolume(m_gameConfig.sfxVolume);
    m_input.Load(m_fileStore);

    // Discover installed datapacks for the selection screen.
    m_registry.Scan(m_fileStore);

    // Init initial state
    m_currentState = std::make_unique<MenuState>();
    m_currentState->OnEnter(*this);
}

Game::~Game() {
    if (m_currentState)
        m_currentState->OnExit(*this);

    CloseAudioDevice();
    CloseWindow();
    m_profiler.Print();
}

// Run
void Game::Run() {
    while (!WindowShouldClose() && m_running) {
        const float dt = GetFrameTime();

        m_screen.OnResize();

        // Update
        m_profiler.Begin("Update");
            m_soundSystem.Tick(dt);
            m_input.Update(m_screen);
            m_currentState->ProcessInput(*this, dt);
            m_currentState->Update(*this, dt);
        m_profiler.End("Update");

        // Draw
        m_profiler.Begin("Draw");
            m_screen.BeginFrame();
                m_currentState->Draw(*this);
            m_screen.EndFrame();
        m_profiler.End("Draw");

        // Swap State
        ApplyPendingState();
    }
}

std::filesystem::path Game::SearchFolderParentPath(const std::string& folderName, size_t searchDepth){
    std::filesystem::path currentDir = GetApplicationDirectory();
    std::cout << "Searching for '" << folderName << "' folder starting from " << currentDir << "\n";

    for (size_t i = 0; i < searchDepth; i++) {
        std::filesystem::path potentialPath = currentDir / folderName;

        if (std::filesystem::exists(potentialPath) && std::filesystem::is_directory(potentialPath)) {
            std::cout << "Found asset directory: " << potentialPath << "\n";
            return potentialPath;
        }

        currentDir = currentDir.parent_path();
    }

    throw std::runtime_error("Resources: could not find directory '" + folderName + "' within " + std::to_string(searchDepth) + " levels up");
    return "NotFound";
}

// Asset loading
void Game::LoadResources() {
    // The global "resources" folder is the lowest-priority (fallback) search root
    // and holds only engine/menu-level assets. Gameplay assets live in datapacks
    // and are mounted on top of this root when a pack is activated.
    m_resources.SetAssetPath(SearchFolderParentPath("resources", 5));
    m_soundSystem.Init(m_resources);
}

// Datapack lifecycle
void Game::ActivateDatapack(const Datapack& pack) {
    // Switching packs: tear the current one down first so nothing leaks or stacks.
    if (m_packActive)
        DeactivateDatapack();

    m_activeDataDir = pack.DataDir();

    // Mount the pack's resources as the highest-priority search root so its assets
    // shadow any global asset of the same key, then load them (tracking keys for unload).
    std::string resourcesRoot =
        (std::filesystem::path(m_fileStore.GetRootPath()) / pack.ResourcesDir()).string();
    m_resources.PushSearchPath(resourcesRoot);
    m_packTextureKeys = m_resources.LoadTexturesFromDir("textures");
    m_packSoundKeys   = m_resources.LoadSoundsFromDir("sounds");
    m_packMusicKeys   = m_resources.LoadMusicFromDir("music");

    // Load gameplay data from the pack. Presets first: the factories resolve
    // emitter preset names while building their templates.
    m_emitterPresets.Load(m_fileStore, m_activeDataDir);
    m_gameData.Load(m_fileStore, m_activeDataDir);
    m_towerFactory.Load(m_fileStore, m_emitterPresets, m_activeDataDir);
    m_enemyFactory.Load(m_fileStore, m_emitterPresets, m_activeDataDir);

    m_packActive = true;
}

void Game::DeactivateDatapack() {
    if (!m_packActive) return;

    // Stop any pack music before freeing the streams it points into.
    m_soundSystem.StopMusic();

    for (const auto& key : m_packTextureKeys) m_resources.UnloadTextureKey(key);
    for (const auto& key : m_packSoundKeys)   m_resources.UnloadSoundKey(key);
    for (const auto& key : m_packMusicKeys)   m_resources.UnloadMusicKey(key);
    m_packTextureKeys.clear();
    m_packSoundKeys.clear();
    m_packMusicKeys.clear();

    // Unmount the pack's resource root (the global base root stays).
    m_resources.PopSearchPath();

    // Drop loaded templates/presets.
    m_towerFactory.Clear();
    m_enemyFactory.Clear();
    m_emitterPresets.Clear();

    m_activeDataDir.clear();
    m_packActive = false;
}

// State machine
void Game::ChangeState(std::unique_ptr<GameState> newState) {
    // Store for deferred application — never swap mid-frame
    m_pendingState = std::move(newState);
}

// Helpers
void Game::ApplyPendingState() {
    if (!m_pendingState) return;

    if (m_currentState)
        m_currentState->OnExit(*this);

    m_currentState = std::move(m_pendingState);
    m_currentState->OnEnter(*this);
}