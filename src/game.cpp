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

    m_gameData.Load(m_fileStore);
    m_emitterPresets.Load(m_fileStore);
    m_towerFactory.Load(m_fileStore, m_emitterPresets);
    m_enemyFactory.Load(m_fileStore, m_emitterPresets);

    LoadResources();
    m_soundSystem.SetMusicVolume(m_gameConfig.musicVolume);
    m_soundSystem.SetSfxVolume(m_gameConfig.sfxVolume);
    m_input.Load(m_fileStore);

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
    // Walk up the directory tree to find the "resources" folder
    m_resources.SetAssetPath(SearchFolderParentPath("resources", 5));

    // Load all images from the textures folder; key = filename stem
    m_resources.LoadTexturesFromDir("textures");
    // Load all music from the music folder; key = filename stem
    m_resources.LoadMusicFromDir("music");
    // Load all sound effects from the sounds folder; key = filename stem
    m_resources.LoadSoundsFromDir("sounds");
    m_soundSystem.Init(m_resources);
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