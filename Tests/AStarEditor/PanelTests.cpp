#include <imgui.h>
#include <imgui_internal.h>
#include "AStarDemoPanel.h"
#include <functional>
#include <climits>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

namespace {
std::map<ImGuiID, ImRect> rectangles;
std::map<std::string, ImGuiID> items;
int passed = 0, failed = 0;
void Require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
struct Frame {
    std::string text;
    Frame() {
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.ClipboardUserData = &text;
        io.SetClipboardTextFn = [](void* user, const char* value) {
            *static_cast<std::string*>(user) = value;
        };
        io.DisplaySize = ImVec2(1800, 1400);
        io.DeltaTime = 1.0f / 60;
        unsigned char* pixels; int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        GImGui->TestEngineHookItems = true;
    }
    ~Frame() { ImGui::DestroyContext(); }
    void Draw(AStarDemoPanel& panel) {
        rectangles.clear(); items.clear();
        ImGui::NewFrame();
        // ImGui flushes the text log when the panel ends. Capture through its
        // clipboard callback in memory, without touching the system clipboard.
        ImGui::LogToClipboard();
        panel.Draw();
        ImGui::LogFinish();
        ImGui::Render();
    }
    void Warm(AStarDemoPanel& panel) { Draw(panel); Draw(panel); Draw(panel); }
    ImRect Item(const char* label) {
        Require(items.count(label) != 0, std::string("missing item: ") + label);
        Require(rectangles.count(items.at(label)) != 0, "item has no rectangle");
        return rectangles.at(items.at(label));
    }
    void Click(AStarDemoPanel& panel, const char* label) {
        const ImRect rect = Item(label);
        auto& io = ImGui::GetIO();
        io.AddMousePosEvent(rect.Min.x + 10, (rect.Min.y + rect.Max.y) / 2);
        Draw(panel);
        io.AddMouseButtonEvent(0, true); Draw(panel);
        io.AddMouseButtonEvent(0, false); Draw(panel);
    }
    void Edit(AStarDemoPanel& panel, const char* label, const char* value, bool ctrlClick = false) {
        auto& io = ImGui::GetIO();
        if (ctrlClick) io.AddKeyEvent(ImGuiMod_Ctrl, true);
        Click(panel, label);
        io.AddKeyEvent(ImGuiMod_Ctrl, true);
        io.AddKeyEvent(ImGuiKey_A, true); Draw(panel);
        io.AddKeyEvent(ImGuiKey_A, false);
        io.AddKeyEvent(ImGuiMod_Ctrl, false); Draw(panel);
        io.AddInputCharactersUTF8(value); Draw(panel);
        io.AddKeyEvent(ImGuiKey_Enter, true); Draw(panel);
        io.AddKeyEvent(ImGuiKey_Enter, false); Draw(panel);
    }
    void Resize(AStarDemoPanel& panel, const char* width, const char* height) {
        Edit(panel, "Width", width); Edit(panel, "Height", height); Click(panel, "Apply");
    }
    void MoveAway(AStarDemoPanel& panel) {
        ImGui::GetIO().AddMousePosEvent(1600, 1200); Draw(panel);
    }
    bool Color(ImU32 color, const ImRect* rect = nullptr) {
        auto* data = ImGui::GetDrawData();
        for (int i = 0; i < data->CmdListsCount; ++i)
            for (const auto& vertex : data->CmdLists[i]->VtxBuffer)
                if (vertex.col == color && (!rect || (vertex.pos.x >= rect->Min.x &&
                    vertex.pos.y >= rect->Min.y && vertex.pos.x <= rect->Max.x && vertex.pos.y <= rect->Max.y)))
                    return true;
        return false;
    }
};
void ValidPath(const AStarDemoModel& model) {
    const auto& path = model.Path();
    Require(!path.empty(), "expected path");
    Require(path.front() == model.Start() && path.back() == model.Goal(), "wrong endpoints");
    for (size_t i = 0; i < path.size(); ++i) {
        const auto cell = path[i];
        Require(cell.x >= 0 && cell.x < model.Width() && cell.y >= 0 && cell.y < model.Height(), "out of bounds");
        Require(model.Map()[int(cell.y)][int(cell.x)] == 0, "path on obstacle");
        if (i) Require((cell - path[i-1]).LengthSquared() > 0 && (cell - path[i-1]).LengthSquared() <= 2,
                       "nonadjacent step");
    }
}
void Run(const char* name, const std::function<void()>& test) {
    try { test(); ++passed; std::cout << "PASS " << name << '\n'; }
    catch (const std::exception& error) {
        ++failed; std::cout << "FAIL " << name << ": " << error.what() << '\n';
    }
}
}

// Observe real ImGui items; no product widgets or state are substituted.
void ImGuiTestEngineHook_ItemAdd(ImGuiContext*, ImGuiID id, const ImRect& bb, const ImGuiLastItemData*) {
    rectangles[id] = bb;
}
void ImGuiTestEngineHook_ItemInfo(ImGuiContext*, ImGuiID id, const char* label, ImGuiItemStatusFlags) {
    items[label] = id;
}
void ImGuiTestEngineHook_Log(ImGuiContext*, const char*, ...) {}
const char* ImGuiTestEngine_FindItemDebugLabel(ImGuiContext*, ImGuiID) { return "editor test"; }

int main() {
    Run("readable default text in real ImGui frame", [] {
        Frame ui; AStarDemoPanel panel; ui.Draw(panel);
        auto* window = ImGui::FindWindowByName(panel.GetName());
        Require(window && window->FontWindowScale >= 1.3f, "default panel text is too small");
    });
    Run("real resize and text scale controls exist", [] {
        Frame ui; AStarDemoPanel panel; ui.Draw(panel);
        for (const char* label : {"Width", "Height", "Apply", "Text scale"})
            Require(items.count(label) != 0, std::string("missing interactive control: ") + label);
    });
    Run("model resize preserves overlap and clears new goal", [] {
        AStarDemoModel model;
        Require(model.Resize(3, 3), "resize rejected");
        Require(model.Map()[0][2] == 1 && model.Map()[1][2] == 1 && model.Map()[2][2] == 0,
                "overlap or protected new goal lost");
        ValidPath(model);
        Require(model.Resize(7, 6), "expand rejected");
        Require(model.Map()[0][2] == 1 && model.Map()[5][6] == 0 && model.Map()[0][6] == 0,
                "expansion failed to preserve overlap/clear new cells");
        ValidPath(model);
    });
    Run("model singleton and narrow endpoint protection", [] {
        AStarDemoModel model;
        Require(model.Resize(1, 1), "1x1 rejected");
        ValidPath(model); Require(model.Path().size() == 1, "not singleton");
        Require(!model.Toggle(0, 0), "singleton blocked");
        Require(model.Resize(1, 8), "1x8 rejected"); ValidPath(model);
        Require(!model.Toggle(0, 7), "vertical goal blocked");
        Require(model.Resize(8, 1), "8x1 rejected"); ValidPath(model);
        Require(!model.Toggle(7, 0) && !model.Toggle(0, 0), "endpoint blocked");
    });
    Run("model invalid sizes and cells leave state intact", [] {
        AStarDemoModel model; const auto map = model.Map(); const auto path = model.Path();
        for (const auto size : {std::pair{0,5}, {-1,5}, {21,5}, {5,0}, {5,-1}, {5,21}, {INT_MAX,INT_MAX}})
            Require(!model.Resize(size.first, size.second), "invalid size accepted");
        for (const auto cell : {std::pair{-1,0}, {0,-1}, {5,0}, {0,5}, {INT_MAX,INT_MAX}})
            Require(!model.Toggle(cell.first, cell.second), "invalid cell accepted");
        Require(model.Map() == map && model.Path() == path, "invalid edits mutated state");
    });
    Run("model obstacle click recomputes no-path and restoration", [] {
        AStarDemoModel model; model.Resize(3, 1); model.Toggle(1, 0);
        Require(model.Path().empty() && model.Map()[0][1] == 1, "stale path after blocking corridor");
        model.Toggle(1, 0); ValidPath(model);
        Require(model.Path().size() == 3, "path not restored");
    });
    Run("model full bounds and old endpoint becomes editable", [] {
        AStarDemoModel model; model.Resize(1, 1); model.Resize(20, 20);
        Require(model.Toggle(1, 1) && !model.Toggle(19, 19), "wrong endpoint protection");
        ValidPath(model);
        model.Resize(3, 3); model.Resize(4, 4);
        Require(model.Toggle(2, 2), "old goal incorrectly protected"); ValidPath(model);
    });
    Run("real input staging validation and apply", [] {
        Frame ui; AStarDemoPanel panel; ui.Warm(panel);
        ui.Edit(panel, "Width", "3");
        Require(ui.text.find("Map: 5 x 5") != std::string::npos, "size changed before Apply");
        ui.Edit(panel, "Height", "1"); ui.Click(panel, "Apply");
        Require(ui.text.find("Map: 3 x 1") != std::string::npos, "Apply not connected to dimensions");
        for (const char* invalid : {"0", "-1", "21"}) {
            ui.Edit(panel, "Width", invalid); ui.Click(panel, "Apply");
            Require(ui.text.find("Map: 3 x 1") != std::string::npos, "invalid size applied");
            Require(ui.text.find("must be 1..20") != std::string::npos, "missing validation message");
        }
        ui.Edit(panel, "Width", "3");
        for (const char* invalid : {"0", "-1", "21"}) {
            ui.Edit(panel, "Height", invalid); ui.Click(panel, "Apply");
            Require(ui.text.find("Map: 3 x 1") != std::string::npos &&
                    ui.text.find("must be 1..20") != std::string::npos, "invalid height applied or not reported");
        }
    });
    Run("real cell clicks clear stale stars and restore path", [] {
        Frame ui; AStarDemoPanel panel; ui.Warm(panel); ui.Resize(panel, "3", "1");
        // Clear the preserved wall at the new goal by resizing; interior starts open.
        Require(ui.text.find("Steps: 2") != std::string::npos, "unexpected initial corridor");
        ui.Click(panel, "Cell 1,0");
        Require(!ui.Color(IM_COL32(180,125,25,255)), "stale gold path vertices after edit");
        auto cell = ui.Item("Cell 1,0");
        Require(ui.Color(IM_COL32(90,95,105,255), &cell), "obstacle not drawn at clicked cell");
        // A tooltip is its own ImGui window and finishes text logging early.
        // Move off it to inspect the main panel's status; inspect stars above
        // in the actual click frame so stale rendering cannot hide for a frame.
        ui.MoveAway(panel);
        Require(ui.text.find("No path found.") != std::string::npos, "click did not block corridor");
        ui.Click(panel, "Cell 1,0");
        Require(ui.Color(IM_COL32(180,125,25,255)), "restored path not drawn in click frame");
        ui.MoveAway(panel);
        Require(ui.text.find("Steps: 2") != std::string::npos && ui.Color(IM_COL32(180,125,25,255)), "path not restored");
        ui.Click(panel, "Cell 0,0"); ui.Click(panel, "Cell 2,0");
        ui.MoveAway(panel);
        Require(ui.text.find("Steps: 2") != std::string::npos, "endpoint click changed path");
        ui.Resize(panel, "1", "1"); ui.Click(panel, "Cell 0,0");
        ui.MoveAway(panel);
        Require(ui.text.find("Steps: 0") != std::string::npos && items.count("Cell 1,0") == 0,
                "singleton wrong or stale resized cells");
    });
    Run("real large grid scrollbars and mouse wheel", [] {
        Frame ui; AStarDemoPanel panel; ui.Warm(panel); ui.Resize(panel, "20", "20"); ui.Draw(panel);
        ImGuiWindow* grid = nullptr;
        for (auto* window : GImGui->Windows)
            if (window->ParentWindow == ImGui::FindWindowByName(panel.GetName()) &&
                (window->Flags & ImGuiWindowFlags_ChildWindow)) grid = window;
        Require(grid && grid->ScrollMax.x > 0 && grid->ScrollMax.y > 0, "large grid is not scrollable on both axes");
        auto& io = ImGui::GetIO();
        io.AddMousePosEvent(grid->InnerRect.Min.x + 30, grid->InnerRect.Min.y + 30); ui.Draw(panel);
        io.AddMouseWheelEvent(-3, 0); ui.Draw(panel); ui.Draw(panel);
        Require(grid->Scroll.x > 0, "horizontal wheel did not scroll actual grid");
        io.AddMouseWheelEvent(0, -3); ui.Draw(panel); ui.Draw(panel);
        Require(grid->Scroll.y > 0, "vertical wheel did not scroll actual grid");
    });
    Run("real text scale slider is bounded", [] {
        Frame ui; AStarDemoPanel panel; ui.Warm(panel);
        auto rect = ui.Item("Text scale"); auto& io = ImGui::GetIO();
        io.AddMousePosEvent(rect.Min.x + 20, (rect.Min.y + rect.Max.y)/2); ui.Draw(panel);
        io.AddMouseButtonEvent(0,true); ui.Draw(panel);
        io.AddMousePosEvent(rect.Max.x + 1000, (rect.Min.y + rect.Max.y)/2); ui.Draw(panel);
        io.AddMouseButtonEvent(0,false); ui.Draw(panel); ui.Draw(panel);
        auto* window = ImGui::FindWindowByName(panel.GetName());
        Require(window->FontWindowScale == 2.5f, "scale maximum not bounded");
        rect = ui.Item("Text scale");
        io.AddMousePosEvent(rect.Min.x + 20, (rect.Min.y + rect.Max.y)/2); ui.Draw(panel);
        io.AddMouseButtonEvent(0,true); ui.Draw(panel);
        io.AddMousePosEvent(rect.Min.x - 1000, (rect.Min.y + rect.Max.y)/2); ui.Draw(panel);
        io.AddMouseButtonEvent(0,false); ui.Draw(panel); ui.Draw(panel);
        Require(window->FontWindowScale == 1.0f, "scale minimum not bounded");
    });
    Run("real text scale Ctrl-click entry is bounded", [] {
        Frame ui; AStarDemoPanel panel; ui.Warm(panel);
        ui.Edit(panel, "Text scale", "99", true); ui.Draw(panel);
        auto* window = ImGui::FindWindowByName(panel.GetName());
        Require(window->FontWindowScale == 2.5f, "typed scale maximum not bounded");
        ui.Edit(panel, "Text scale", "-10", true); ui.Draw(panel);
        Require(window->FontWindowScale == 1.0f, "typed scale minimum not bounded");
    });
    std::cout << "RESULT " << passed << " passed, " << failed << " failed\n";
    return failed ? 1 : 0;
}
