#pragma once

#include <states/game_state.hpp>
#include <engine/features/ui_widgets.hpp>

class MenuState : public GameState {
public:
    void OnEnter(Game& game) override;
    void OnExit(Game& game) override;

    void ProcessInput(Game& game, float dt) override;
    void Update(Game& game, float dt) override;
    void Draw(Game& game) override;

private:
    Button m_playButton;
    Button m_settingsButton;
    Button m_particleEditorButton;
    Button m_exitButton;
};