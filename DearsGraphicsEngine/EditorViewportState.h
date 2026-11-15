#pragma once

#include <cmath>
#include <limits>
#include <optional>

// ImGui/Win32 use client pixels in this single-window editor. No desktop origin
// or inspector width belongs in a picking ray; only the displayed image does.
struct EditorViewportState
{
    struct Point { float x = 0; float y = 0; };
    float x = 0, y = 0, width = 0, height = 0;
    bool hovered = false, focused = false, textInput = false;

    bool Valid() const
    {
        return std::isfinite(x) && std::isfinite(y) && std::isfinite(width) && std::isfinite(height)
            && width >= 1 && height >= 1 && width < float((std::numeric_limits<int>::max)())
            && height < float((std::numeric_limits<int>::max)());
    }
    int PixelWidth() const { return Valid() ? int(width) : 0; }
    int PixelHeight() const { return Valid() ? int(height) : 0; }
    std::optional<Point> ToLocal(float mouseX, float mouseY) const
    {
        if (!Valid() || !std::isfinite(mouseX) || !std::isfinite(mouseY)
            || mouseX < x || mouseY < y || mouseX >= x + width || mouseY >= y + height) return {};
        return Point{(mouseX - x) * PixelWidth() / width, (mouseY - y) * PixelHeight() / height};
    }
    bool AllowsSceneInput(float mouseX, float mouseY) const
    {
        return hovered && focused && !textInput && ToLocal(mouseX, mouseY).has_value();
    }
};

// WM_SIZE queues a client extent; the frame loop applies it outside WndProc.
// Minimize never requests a zero-sized GPU resource or destroys the last extent.
struct EditorClientSize
{
    int width = 0, height = 0;
    bool minimized = true, pending = false;
    void Notify(int newWidth, int newHeight, bool isMinimized)
    {
        const bool wasMinimized = minimized;
        minimized = isMinimized || newWidth <= 0 || newHeight <= 0;
        if (minimized) { pending = false; return; }
        pending = pending || wasMinimized || newWidth != width || newHeight != height;
        width = newWidth; height = newHeight;
    }
};
