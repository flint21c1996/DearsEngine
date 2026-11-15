#pragma once

#include <functional>
#include <string>
#include <vector>
#include <imgui.h>
#include "EditorViewportState.h"

// A normal movable ImGui window. Docking is deliberately not emulated here.
// Keep it open until the world overlay has been submitted to this draw list.
class EditorViewportWindow
{
public:
    EditorViewportState state;
    ImVec2 canvasOrigin{};
    bool showOverlay = false;

    // Drag-and-drop onto the world image. Payloads are null-terminated strings
    // (an asset or preset name); x/y are pixels inside the image.
    struct Drop
    {
        bool pending = false;
        std::string type;
        std::string data;
        float x = 0, y = 0;
    };
    std::vector<std::string> dropTypes;
    Drop drop;

    void Begin(const std::function<void*(int, int)>& ensureSurface)
    {
        const auto display = ImGui::GetIO().DisplaySize;
        const float dpi = ImGui::GetIO().FontGlobalScale;
        ImGui::SetNextWindowPos(ImVec2(12 * dpi, 32 * dpi), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(display.x * 0.68f, display.y * 0.85f), ImGuiCond_FirstUseEver);
        state = {};
        m_clipPushed = false;
        if (!ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) return;
        ImGui::TextUnformatted("RMB + WASD/QE: navigate");
        ImGui::SameLine();
        ImGui::Checkbox("Debug overlay", &showOverlay);
        const auto origin = ImGui::GetCursorScreenPos();
        canvasOrigin = ImGui::GetCursorPos();
        const auto size = ImGui::GetContentRegionAvail();
        EditorViewportState image{origin.x, origin.y, size.x, size.y};
        if (!image.Valid()) return;
        void* texture = ensureSurface(image.PixelWidth(), image.PixelHeight());
        if (!texture) { ImGui::TextUnformatted("Viewport surface unavailable."); return; }
        ImGui::Image(texture, size);
        // Assets dragged from editor panels can be dropped onto the world image.
        // The window only records what landed where; the game layer decides what it means.
        if (!dropTypes.empty() && ImGui::BeginDragDropTarget())
        {
            for (const std::string& type : dropTypes)
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type.c_str()))
                {
                    const ImVec2 mouse = ImGui::GetMousePos();
                    if (const auto local = image.ToLocal(mouse.x, mouse.y))
                    {
                        drop.pending = true;
                        drop.type = type;
                        drop.data = payload->Data ? std::string(static_cast<const char*>(payload->Data)) : std::string();
                        drop.x = local->x;
                        drop.y = local->y;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        // Image has no active widget ID: explicitly transfer focus on the click
        // before the engine consumes the same-frame LMB/RMB event.
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right))
            ImGui::SetWindowFocus();
        image.hovered = ImGui::IsItemHovered();
        image.focused = ImGui::IsWindowFocused();
        image.textInput = ImGui::GetIO().WantTextInput;
        state = image;
        ImGui::PushClipRect(origin, ImVec2(origin.x + size.x, origin.y + size.y), true);
        m_clipPushed = true;
    }
    void End()
    {
        if (m_clipPushed) ImGui::PopClipRect();
        ImGui::End();
    }
private:
    bool m_clipPushed = false;
};
