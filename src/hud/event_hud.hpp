#pragma once

#include <hud/hud.hpp>
#include <string>
#include <vector>

class Game;

class EventHUD : public HUD {
public:
    // Add a message; adding the same message as the newest entry resets its timer
    void Add(const std::string& message, float duration = 3.0f);
    void Update(Game& game, float dt) override;

protected:
    void OnDraw(Game& game) override;

private:
    static constexpr int   kMaxEntries = 5;
    static constexpr float FADE_TIME   = 1.0f; // seconds over which entries fade out

    struct Entry {
        std::string message;
        float timeLeft;
    };

    std::vector<Entry> m_entries;
};
