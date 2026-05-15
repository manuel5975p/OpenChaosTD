#include <hud/event_log.hpp>
#include <raylib.h>
#include <algorithm>

void EventLog::Add(const std::string& message, float duration) {
    // Reset timer if this message is already the newest — prevents visual spam
    if (!m_entries.empty() && m_entries.back().message == message) {
        m_entries.back().timeLeft = duration;
        return;
    }

    // Drop oldest entry when at capacity
    if (static_cast<int>(m_entries.size()) >= MAX_ENTRIES)
        m_entries.erase(m_entries.begin());

    m_entries.push_back({message, duration, duration});
}

void EventLog::Update(float dt) {
    for (auto& entry : m_entries)
        entry.timeLeft -= dt;

    std::erase_if(m_entries, [](const Entry& e) { return e.timeLeft <= 0.0f; });
}

void EventLog::Draw() {
    if (m_entries.empty()) return;

    constexpr float MARGIN      = 8.0f;
    constexpr float LINE_H      = 20.0f;
    constexpr float SCORE_HUD_H = 36.0f; // keep messages below the top panel

    int n = static_cast<int>(m_entries.size());
    float baseY = SCORE_HUD_H + MARGIN;

    for (int i = 0; i < n; i++) {
        const Entry& entry = m_entries[i];

        // Fade alpha during the last FADE_TIME seconds
        float t = std::min(entry.timeLeft / FADE_TIME, 1.0f);
        unsigned char textAlpha = static_cast<unsigned char>(t * 220.0f);
        unsigned char bgAlpha   = static_cast<unsigned char>(t * 160.0f);

        // Stack downward: oldest (index 0) at baseY, newer entries below it
        float y = baseY + static_cast<float>(i) * LINE_H;

        int textW = MeasureText(entry.message.c_str(), 12);
        DrawRectangleRec({MARGIN - 2, y - 1, static_cast<float>(textW + 10), LINE_H - 2}, {20, 20, 20, bgAlpha});
        DrawText(entry.message.c_str(), static_cast<int>(MARGIN + 3), static_cast<int>(y + 2), 12, {255, 220, 80, textAlpha});
    }
}
