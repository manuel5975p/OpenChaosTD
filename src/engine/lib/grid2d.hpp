#pragma once

#include <algorithm>
#include <vector>
#include <cassert>

template<typename T>
class Grid2D {
public:
    Grid2D() = default;

    // width > 0, height > 0 (positive dimensions are a caller invariant).
    Grid2D(int width, int height, const T& fill = T{})
        : m_width(width)
        , m_height(height)
        , m_data(width * height, fill)
    {
        assert(width > 0 && height > 0 && "Grid2D: dimensions must be positive");
    }

    // Resize — resets all cells to fill value. Pre: width > 0, height > 0.
    void Resize(int width, int height, const T& fill = T{}) {
        assert(width > 0 && height > 0 && "Grid2D: dimensions must be positive");
        m_width  = width;
        m_height = height;
        m_data.assign(width * height, fill);
    }

    // Access
    std::vector<T>& GetVector() {
        return m_data;
    }

    const std::vector<T>& GetVector() const {
        return m_data;
    }

    T& Get(int x, int y) {
        CheckBounds(x, y);
        return m_data[y * m_width + x];
    }

    const T& Get(int x, int y) const {
        CheckBounds(x, y);
        return m_data[y * m_width + x];
    }

    void Set(int x, int y, const T& value) {
        CheckBounds(x, y);
        m_data[y * m_width + x] = value;
    }

    // Query
    bool InBounds(int x, int y) const {
        return x >= 0 && x < m_width && y >= 0 && y < m_height;
    }

    int GetWidth()  const { return m_width;  }
    int GetHeight() const { return m_height; }
    int GetSize()   const { return m_width * m_height; }

    // Fill entire grid
    void Fill(const T& value) {
        std::fill(m_data.begin(), m_data.end(), value);
    }

    // Iterators — allows range-based for loops
    typename std::vector<T>::iterator begin() { return m_data.begin(); }
    typename std::vector<T>::iterator end() { return m_data.end(); }
    typename std::vector<T>::const_iterator begin() const { return m_data.begin(); }
    typename std::vector<T>::const_iterator end() const { return m_data.end(); }

private:
    void CheckBounds(int x, int y) const {
        assert(InBounds(x, y) && "Grid2D: index out of range");
    }

    int m_width  = 0;
    int m_height = 0;
    std::vector<T> m_data;
};