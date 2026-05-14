#pragma once

#include <states/game_state.hpp>
#include <core/button.hpp>

class MenuState : public GameState {
public:
    void OnEnter(Game& game) override;
    void OnExit(Game& game) override;

    void ProcessInput(Game& game, float dt) override;
    void Update(Game& game, float dt) override;
    void Draw(Game& game) override;

private:
    Button m_playButton;
    Button m_exitButton;
};