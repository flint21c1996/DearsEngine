#include "Camera.h"
#include "EditorViewportWindow.h"
#include <imgui_internal.h>
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
int passed = 0, failed = 0;
void Require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
void Near(float actual, float expected) { Require(std::abs(actual - expected) < 0.01f, "coordinate mismatch"); }
void Run(const char* name, const std::function<void()>& test)
{
    try { test(); ++passed; std::cout << "PASS " << name << '\n'; }
    catch (const std::exception& error) { ++failed; std::cout << "FAIL " << name << ": " << error.what() << '\n'; }
}
struct Gui
{
    EditorViewportWindow viewport;
    int requests = 0, requestedWidth = 0, requestedHeight = 0;
    bool inspector = false;
    bool surfaceAvailable = true;
    ImVec2 inputMin{}, inputMax{};
    Gui(float width = 1920, float height = 1080)
    {
        ImGui::CreateContext();
        auto& io = ImGui::GetIO(); io.IniFilename = nullptr;
        io.DisplaySize = ImVec2(width, height); io.DeltaTime = 1.0f / 60;
        unsigned char* pixels; int w, h; io.Fonts->GetTexDataAsRGBA32(&pixels, &w, &h);
    }
    ~Gui() { ImGui::DestroyContext(); }
    void Draw()
    {
        ImGui::NewFrame();
        viewport.Begin([this](int width, int height) -> void* {
            ++requests; requestedWidth = width; requestedHeight = height;
            return surfaceAvailable ? reinterpret_cast<void*>(1) : nullptr; // borrowed external texture, never dereferenced without a renderer
        });
        if (inspector)
        {
            ImGui::SetNextWindowPos(ImVec2(30, 90)); ImGui::SetNextWindowSize(ImVec2(300, 200));
            ImGui::Begin("Inspector");
            static char text[64]{}; ImGui::InputText("Name", text, sizeof(text));
            inputMin = ImGui::GetItemRectMin(); inputMax = ImGui::GetItemRectMax();
            ImGui::End();
        }
        viewport.End(); ImGui::Render();
    }
    void Warm() { Draw(); Draw(); Draw(); }
};
}

int main()
{
    Run("valid image dimensions update camera aspect", [] {
        Camera camera(1920,1080); camera.SetAspectRatio(640,480); Near(camera.GetAspectRatio(),4.0f/3);
    });
    Run("minimized dimensions preserve a valid camera aspect", [] {
        Camera camera(640,480); camera.SetAspectRatio(0,0);
        Require(std::isfinite(camera.GetAspectRatio()) && camera.GetAspectRatio() > 0,"invalid aspect");
        Near(camera.GetAspectRatio(),4.0f/3);
    });
    Run("negative dimensions preserve a valid camera aspect", [] {
        Camera camera(640,480); camera.SetAspectRatio(-1,480);
        Require(std::isfinite(camera.GetAspectRatio()) && camera.GetAspectRatio() > 0,"invalid aspect");
        Near(camera.GetAspectRatio(),4.0f/3);
    });
    Run("pixel orthographic resize tracks image while explicit world extents stay fixed", [] {
        Camera camera(640,480); camera.SetOrthgraphic(0.1f); camera.SetAspectRatio(1280,720);
        Near(camera.GetOrthographicWidth()/camera.GetOrthographicHeight(),1280.0f/720);
        camera.SetOrthographicSize(40,30); camera.SetAspectRatio(640,640);
        Near(camera.GetOrthographicWidth(),40); Near(camera.GetOrthographicHeight(),30);
    });
    Run("moved and fractional image coordinates agree with actual projection", [] {
        for (auto rect : {EditorViewportState{120,85,1000,600}, EditorViewportState{640,230,1280.5f,700.5f}})
        {
            Camera camera(rect.PixelWidth(),rect.PixelHeight()); camera.ProjectionSettings(60,0.1f,100);
            const auto projected = DirectX::XMVector3Project(DirectX::XMVectorSet(1,0,5,1),0,0,
                float(rect.PixelWidth()),float(rect.PixelHeight()),0,1,camera.GetProjRow(),camera.GetViewRow(),Matrix::Identity);
            const float px=DirectX::XMVectorGetX(projected), py=DirectX::XMVectorGetY(projected);
            auto local=rect.ToLocal(rect.x+px*rect.width/rect.PixelWidth(),rect.y+py*rect.height/rect.PixelHeight());
            Require(local.has_value(),"projected point not in image"); Near(local->x,px); Near(local->y,py);
        }
    });
    Run("bounds and zero or nonfinite extents reject input", [] {
        EditorViewportState rect{100,50,800,450};
        Require(rect.ToLocal(100,50).has_value(),"top-left excluded");
        for (auto p : {EditorViewportState::Point{99,50},{100,49},{900,200},{500,500}})
            Require(!rect.ToLocal(p.x,p.y),"exterior accepted");
        for (float bad : {0.0f,-1.0f,0.5f,(std::numeric_limits<float>::infinity)(),(std::numeric_limits<float>::quiet_NaN)()})
        { rect.width=bad; Require(!rect.Valid() && !rect.ToLocal(100,50),"invalid extent accepted"); }
    });
    Run("scene input requires hovered focused image and no text entry", [] {
        EditorViewportState rect{100,50,800,450,true,true,false};
        Require(rect.AllowsSceneInput(200,200),"valid image input blocked");
        rect.hovered=false; Require(!rect.AllowsSceneInput(200,200),"inspector hover leaked");
        rect.hovered=true; rect.focused=false; Require(!rect.AllowsSceneInput(200,200),"unfocused input leaked");
        rect.focused=true; rect.textInput=true; Require(!rect.AllowsSceneInput(200,200),"text entry leaked");
    });
    Run("client minimize restore and QHD sizing never queues zero resources", [] {
        EditorClientSize client; client.Notify(1920,1080,false); Require(client.pending,"initial resize absent");
        client.pending=false; client.Notify(0,0,true);
        Require(client.minimized && !client.pending && client.width==1920 && client.height==1080,"minimize destroyed extent");
        client.Notify(1920,1080,false); Require(client.pending && !client.minimized,"same-size restore not queued");
        client.pending=false;
        client.Notify(2560,1440,false); Require(!client.minimized && client.pending,"restore resize absent");
        Require(client.width==2560 && client.height==1440,"QHD extent truncated");
        client.pending=false; client.Notify(-1,600,false); Require(client.minimized && !client.pending,"negative resize queued");
    });
    Run("real ImGui image move resize and captured mouse navigation", [] {
        Gui gui; gui.Warm(); ImGui::SetWindowPos("Viewport",ImVec2(180,130)); ImGui::SetWindowSize("Viewport",ImVec2(900,650));
        ImGui::SetWindowFocus("Viewport"); gui.Draw(); gui.Draw();
        const auto& image=gui.viewport.state;
        Require(image.Valid() && image.x>=180 && image.y>=130,"image did not move with window");
        Require(gui.requestedWidth==image.PixelWidth() && gui.requestedHeight==image.PixelHeight(),"resource/image sizes differ");
        auto& io=ImGui::GetIO(); io.AddMousePosEvent(image.x+image.width/2,image.y+image.height/2); gui.Draw(); gui.Draw();
        Require(io.WantCaptureMouse,"test did not exercise ImGui mouse capture");
        Require(gui.viewport.state.AllowsSceneInput(io.MousePos.x,io.MousePos.y),"capture blocked legitimate image input");
        Require(ImGui::GetDrawData()->TotalVtxCount>0,"no actual UI geometry");
    });
    Run("real inspector and active text input protect the scene", [] {
        Gui gui; gui.inspector=true; gui.Warm(); auto& io=ImGui::GetIO();
        io.AddMousePosEvent(gui.inputMin.x+10,(gui.inputMin.y+gui.inputMax.y)/2); gui.Draw();
        io.AddMouseButtonEvent(0,true); gui.Draw(); io.AddMouseButtonEvent(0,false); gui.Draw(); gui.Draw();
        Require(io.WantTextInput,"actual text widget not active");
        const auto image=gui.viewport.state;
        io.AddMousePosEvent(image.x+image.width-30,image.y+image.height/2); gui.Draw(); gui.Draw();
        Require(!gui.viewport.state.AllowsSceneInput(io.MousePos.x,io.MousePos.y),"active inspector text leaked input");
    });
    Run("clicking the image transfers focus from inspector for scene input", [] {
        Gui gui; gui.inspector=true; gui.Warm(); auto& io=ImGui::GetIO();
        const auto image=gui.viewport.state;
        io.AddMousePosEvent(image.x+image.width-30,image.y+image.height/2); gui.Draw(); gui.Draw();
        io.AddMouseButtonEvent(0,true); gui.Draw();
        Require(gui.viewport.state.AllowsSceneInput(io.MousePos.x,io.MousePos.y),"first scene click did not get focus");
        io.AddMouseButtonEvent(0,false); gui.Draw();
    });
    Run("normal window placement persists through actual ImGui settings", [] {
        std::string settings;
        { Gui gui; gui.Warm(); ImGui::SetWindowPos("Viewport",ImVec2(190,140)); ImGui::SetWindowSize("Viewport",ImVec2(850,620)); gui.Draw();
          settings=ImGui::SaveIniSettingsToMemory(); }
        Gui restored; ImGui::LoadIniSettingsFromMemory(settings.c_str()); restored.Warm();
        auto* window=ImGui::FindWindowByName("Viewport");
        Near(window->Pos.x,190); Near(window->Pos.y,140); Near(window->Size.x,850); Near(window->Size.y,620);
    });
    Run("collapsed viewport never asks for a zero surface", [] {
        Gui gui; gui.Warm(); const int before=gui.requests;
        ImGui::SetWindowCollapsed("Viewport",true); gui.Draw();
        Require(!gui.viewport.state.Valid() && gui.requests==before,"collapsed window requested rendering");
        ImGui::SetWindowCollapsed("Viewport",false); gui.Draw(); Require(gui.viewport.state.Valid(),"restore failed");
    });
    Run("unavailable surface disables image input and can recover", [] {
        Gui gui; gui.Warm(); gui.surfaceAvailable=false; gui.Draw();
        Require(!gui.viewport.state.Valid(),"failed allocation left an interactive stale image");
        gui.surfaceAvailable=true; gui.Draw(); Require(gui.viewport.state.Valid(),"surface recovery failed");
    });
    Run("QHD and scaled UI retain correct positive image coordinates", [] {
        Gui gui(2560,1440); ImGui::GetIO().FontGlobalScale=1.5f; ImGui::GetStyle().ScaleAllSizes(1.5f); gui.Warm();
        const auto image=gui.viewport.state; Require(image.PixelWidth()>0 && image.PixelHeight()>0,"scaled image invalid");
        auto center=image.ToLocal(image.x+image.width/2,image.y+image.height/2);
        Require(center.has_value(),"scaled center invalid"); Near(center->x,image.PixelWidth()/2.0f); Near(center->y,image.PixelHeight()/2.0f);
    });
    std::cout << "RESULT " << passed << " passed, " << failed << " failed\n";
    return failed ? 1 : 0;
}
