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

    m_entries.push_back({message, duration});
}

void EventLog::Update(Game& /*game*/, float dt) {
    for (auto& entry : m_entries)
        entry.timeLeft -= dt;

    std::erase_if(m_entries, [](const Entry& e) { return e.timeLeft <= 0.0f; });
}

void EventLog::OnDraw(Game& /*game*/) {
    if (m_entries.empty()) return;

    const float margin    = Scaled(8.0f);
    const float lineH     = Scaled(20.0f);
    const float scoreHudH = Scaled(36.0f); // keep messages below the top panel
    const int   fontSize  = ScaledInt(12.0f);

    int n = static_cast<int>(m_entries.size());
    float baseY = scoreHudH + margin;

    for (int i = 0; i < n; i++) {
        const Entry& entry = m_entries[i];

        // Fade alpha during the last FADE_TIME seconds
        float t = std::min(entry.timeLeft / FADE_TIME, 1.0f);
        unsigned char textAlpha = static_cast<unsigned char>(t * 220.0f);
        unsigned char bgAlpha   = static_cast<unsigned char>(t * 160.0f);

        // Stack downward: oldest (index 0) at baseY, newer entries below it
        float y = baseY + static_cast<float>(i) * lineH;

        int textW = MeasureText(entry.message.c_str(), fontSize);
        DrawRectangleRec({margin - Scaled(2.0f), y - Scaled(1.0f),
                          textW + Scaled(10.0f), lineH - Scaled(2.0f)}, {20, 20, 20, bgAlpha});
        DrawText(entry.message.c_str(), static_cast<int>(margin + Scaled(3.0f)),
                 static_cast<int>(y + Scaled(2.0f)), fontSize, {255, 220, 80, textAlpha});
    }
}
