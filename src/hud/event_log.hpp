#pragma once

#include <hud/hud.hpp>
#include <string>
#include <vector>

class Game;

class EventLog : public HUD {
public:
    void Build(Game& game);
    // Add a message; adding the same message as the newest entry resets its timer
    void Add(const std::string& message, float duration = 3.0f);
    void Update(float dt);
    void Draw();

private:
    static constexpr int   MAX_ENTRIES = 5;
    static constexpr float FADE_TIME   = 1.0f; // seconds over which entries fade out

    struct Entry {
        std::string message;
        float timeLeft;
        float duration;
    };

    std::vector<Entry> m_entries;
};
