#pragma once

#include <cstdio>
#include <string>

// Compact float formatting for serialization: trailing zeros stripped, at least one
// decimal digit kept (2.0, 0.38). Serializing the raw float widened to double would
// otherwise emit 0.3799999952316284-style noise; formatting to a fixed-precision
// decimal string first keeps saved values clean and round-trippable.
inline std::string FormatFloat(float v) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.4f", v);
    std::string s(buf);
    size_t dot = s.find('.');
    size_t last = s.find_last_not_of('0');
    s.erase(last > dot ? last + 1 : dot + 2);
    return s;
}
