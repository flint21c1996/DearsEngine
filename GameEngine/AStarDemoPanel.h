#pragma once

#include <algorithm>
#include <cstdio>
#include <imgui.h>

#include "AStarDemoModel.h"
#include "IEditorPanel.h"

// A scene-owned view of the existing pathfinder, independent of 3D scene assets.
class AStarDemoPanel final : public IEditorPanel
{
public:
    const char* GetName() const override { return "A* Demo"; }

    void Draw() override
    {
        ImGui::SetNextWindowPos(ImVec2(24, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(560, 740), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(GetName())) {
            ImGui::SetWindowFontScale(m_textScale);
            ImGui::SetNextItemWidth(180);
            ImGui::SliderFloat("Text scale", &m_textScale, 1.0f, 2.5f, "%.1fx", ImGuiSliderFlags_AlwaysClamp);
            ImGui::TextUnformatted("S: Start    G: Goal    S/G: Both");
            ImGui::TextWrapped("#: Obstacle    *: Calculated path");
            ImGui::TextWrapped("8-way movement (diagonals allowed)");
            ImGui::TextWrapped("Click cells to toggle obstacles. Start and goal are fixed and protected.");
            ImGui::SetNextItemWidth(160);
            ImGui::InputInt("Width", &m_requestedWidth);
            ImGui::SetNextItemWidth(160);
            ImGui::InputInt("Height", &m_requestedHeight);
            const bool validSize = AStarDemoModel::ValidSize(m_requestedWidth, m_requestedHeight);
            if (!validSize) ImGui::TextUnformatted("Width and height must be 1..20.");
            ImGui::BeginDisabled(!validSize);
            if (ImGui::Button("Apply")) m_model.Resize(m_requestedWidth, m_requestedHeight);
            ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Button("Recalculate")) m_model.Recalculate();
            ImGui::Text("Map: %d x %d (size range 1..20)", m_model.Width(), m_model.Height());
            ImGui::Separator();

            DrawGrid();
            if (m_model.Path().empty()) ImGui::TextUnformatted("No path found.");
            else ImGui::Text("Steps: %d", static_cast<int>(m_model.Path().size()) - 1);
        }
        ImGui::End();
    }

private:
    void DrawGrid()
    {
        const float gridHeight = (std::max)(100.0f, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing());
        if (ImGui::BeginChild("Grid", ImVec2(0, gridHeight), true, ImGuiWindowFlags_HorizontalScrollbar)) {
            const float cellSize = (std::max)(52.0f, ImGui::CalcTextSize("S/G").x + 12.0f);
            const float pitch = cellSize + 4;
            const ImVec2 origin = ImGui::GetCursorScreenPos();
            ImDrawList* draw = ImGui::GetWindowDrawList();
            // Collect input before drawing so a click cannot leave old path stars in this frame.
            for (int y = 0; y < m_model.Height(); ++y) {
                for (int x = 0; x < m_model.Width(); ++x) {
                    ImGui::SetCursorScreenPos(ImVec2(origin.x + x * pitch, origin.y + y * pitch));
                    char id[32];
                    std::snprintf(id, sizeof(id), "Cell %d,%d", x, y);
                    ImGui::BeginDisabled(m_model.IsProtected(x, y));
                    if (ImGui::InvisibleButton(id, ImVec2(cellSize, cellSize))) m_model.Toggle(x, y);
                    ImGui::EndDisabled();
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                        ImGui::SetTooltip("(%d, %d)%s", x, y, m_model.IsProtected(x, y) ? " - protected endpoint" : " - click to toggle");
                    }
                }
            }
            for (int y = 0; y < m_model.Height(); ++y) {
                for (int x = 0; x < m_model.Width(); ++x) {
                    const Vector2 position{float(x), float(y)};
                    const char* marker = ".";
                    ImU32 color = IM_COL32(50, 60, 72, 255);
                    if (m_model.Map()[y][x] == 1) {
                        marker = "#";
                        color = IM_COL32(90, 95, 105, 255);
                    } else if (std::find(m_model.Path().begin(), m_model.Path().end(), position) != m_model.Path().end()) {
                        marker = "*";
                        color = IM_COL32(180, 125, 25, 255);
                    }
                    if (position == m_model.Start()) { marker = "S"; color = IM_COL32(25, 130, 65, 255); }
                    if (position == m_model.Goal()) {
                        marker = position == m_model.Start() ? "S/G" : "G";
                        color = IM_COL32(160, 45, 55, 255);
                    }

                    const ImVec2 topLeft(origin.x + x * pitch, origin.y + y * pitch);
                    draw->AddRectFilled(topLeft, ImVec2(topLeft.x + cellSize, topLeft.y + cellSize), color);
                    const ImVec2 textSize = ImGui::CalcTextSize(marker);
                    draw->AddText(ImVec2(topLeft.x + (cellSize - textSize.x) / 2,
                                        topLeft.y + (cellSize - textSize.y) / 2),
                                  IM_COL32(255, 255, 255, 255), marker);
                }
            }
            ImGui::SetCursorScreenPos(origin);
            ImGui::Dummy(ImVec2(m_model.Width() * pitch, m_model.Height() * pitch));
        }
        ImGui::EndChild();
    }

    AStarDemoModel m_model;
    int m_requestedWidth = 5;
    int m_requestedHeight = 5;
    float m_textScale = 1.5f;
};
