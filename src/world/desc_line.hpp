#pragma once

#include <string>
#include <vector>
#include <cstdarg>
#include <cstdio>
#include <raylib.h>

// One described line: a formatted text row plus the color it should render in.
// Every module's DescribeStats() appends zero or more of these (as does TowerUpgrade).
struct DescLine {
    std::string m_text;
    Color m_color;
};

// Format a single stat row (printf-style) and append it to out. Collapses the
// `char buf[N]; snprintf(...); out.push_back({buf, color});` idiom repeated across
// every tower/enemy module's DescribeStats.
#if defined(__GNUC__)
__attribute__((format(printf, 3, 4)))
#endif
inline void PushStatLine(std::vector<DescLine>& out, Color color, const char* fmt, ...) {
    char buf[64];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    out.push_back({buf, color});
}
