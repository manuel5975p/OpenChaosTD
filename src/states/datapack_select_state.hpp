#pragma once

#include <states/game_state.hpp>
#include <engine/features/ui_widgets.hpp>
#include <raylib.h>

// Scrollable datapack picker reached from the main menu. Lists every installed
// pack (icon, name, author, version, description); selecting one activates it and
// transitions to the screen named by the launch intent.
class DatapackSelectState : public GameState {
public:
    // Where to go once a pack is chosen: into a game, or into the particle editor.
    enum class Intent { Play, EditParticles };

    explicit DatapackSelectState(Intent intent) : m_intent(intent) {}

    void OnEnter(Game& game) override;
    void OnExit(Game& game) override;

    void ProcessInput(Game& game, float dt) override;
    void Update(Game& game, float dt) override;
    void Draw(Game& game) override;

private:
    // On-screen rect of card i, accounting for the current scroll offset.
    Rectangle CardRect(Game& game, int index) const;
    float ListTop() const { return kListTop; }
    float ListBottom(Game& game) const;
    float MaxScroll(Game& game) const;
    // Selecting a pack: activate it and route to the intent's destination state.
    void SelectPack(Game& game, int index);

    Intent m_intent;
    float m_scroll = 0.0f;
    int m_hovered = -1; // index of the card under the cursor, or -1
    Button m_backButton;

    // Layout constants (raw virtual coords, matching the other states' approach).
    static constexpr float kMargin   = 40.0f;
    static constexpr float kListTop   = 110.0f; // below the title
    static constexpr float kFooterH   = 80.0f;  // back-button strip at the bottom
    static constexpr float kCardH     = 120.0f;
    static constexpr float kCardGap   = 12.0f;
    static constexpr float kIconPad   = 16.0f;
    static constexpr float kScrollSpeed = 40.0f; // virtual px per wheel notch
};
