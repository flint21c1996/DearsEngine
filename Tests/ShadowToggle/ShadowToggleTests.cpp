#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <wrl/client.h>
#include <cmath>
#include <cstring>
#include <cstddef>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "BufferData.h"
#include "IDearsRenderer.h"
#include "RenderPipeline.h"
#include "ShadowFrameState.h"

using Microsoft::WRL::ComPtr;

static void Check(bool value, const char* message)
{
    if (!value) throw std::runtime_error(message);
}
static void HR(HRESULT result) { Check(SUCCEEDED(result), "D3D operation failed"); }

static ComPtr<ID3DBlob> Compile(const wchar_t* path, const char* target)
{
    ComPtr<ID3DBlob> code, error;
    HRESULT result = D3DCompileFromFile(path, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", target, D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &code, &error);
    if (error) std::cout << static_cast<const char*>(error->GetBufferPointer());
    HR(result);
    return code;
}

// Uses the real production Forward PBR / Deferred Lighting pixel shaders on WARP.
// Only geometry and the backend adapter are fixtures: known G-buffer, light and
// stale depth textures let the output distinguish ordinary light from shadows.
class GpuRenderer : public IDearsRenderer
{
public:
    RenderPipeline pipeline;
    CommonConstantBufferData common{};
    bool writeMaps = true, finishWrites = true;
    float nextDepth = 0;
    unsigned writes = 0;
    ShadowFrameState<MAX_LIGHTS> shadowFrame;
    unsigned facesToWrite = 6, lightIndex = 0;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<ID3D11Buffer> constants, material, classicMaterial;
    ComPtr<ID3D11Texture2D> output, staging, twoD, cube;
    ComPtr<ID3D11RenderTargetView> target;
    ComPtr<ID3D11ShaderResourceView> twoDSrv, cubeSrv;
    ComPtr<ID3D11PixelShader> forward, deferred, classic;
    ComPtr<ID3D11VertexShader> forwardVs, deferredVs;
    std::vector<ComPtr<ID3D11ShaderResourceView>> gbuffer;

    GpuRenderer()
    {
        D3D_FEATURE_LEVEL level;
        HR(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0,
            D3D11_SDK_VERSION, &device, &level, &context));
        Check(level >= D3D_FEATURE_LEVEL_11_0, "WARP shader model 5 required");
        auto f = Compile(L"DearsGraphicsEngine/Shader/PBRPixelShader.hlsl", "ps_5_0");
        auto d = Compile(L"DearsGraphicsEngine/Shader/DeferredLightingPixelShader.hlsl", "ps_5_0");
        auto c = Compile(L"DearsGraphicsEngine/Shader/PixelShader.hlsl", "ps_5_0");
        CheckLayout(f.Get());
        CheckLayout(d.Get());
        CheckLayout(c.Get());
        HR(device->CreatePixelShader(f->GetBufferPointer(), f->GetBufferSize(), nullptr, &forward));
        HR(device->CreatePixelShader(d->GetBufferPointer(), d->GetBufferSize(), nullptr, &deferred));
        HR(device->CreatePixelShader(c->GetBufferPointer(), c->GetBufferSize(), nullptr, &classic));
        forwardVs = Vertex("Tests/ShadowToggle/ForwardFixture.hlsl");
        deferredVs = Vertex("Tests/ShadowToggle/DeferredFixture.hlsl");
        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = sizeof(common); bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        HR(device->CreateBuffer(&bd, nullptr, &constants));
        PBRPixelShaderConstantData pbr{};
        pbr.material.albedo = Vector3(1); pbr.material.roughness = .5f;
        bd.ByteWidth = sizeof(pbr);
        D3D11_SUBRESOURCE_DATA initial{ &pbr, 0, 0 };
        HR(device->CreateBuffer(&bd, &initial, &material));
        PSConstantBufferData phong{};
        phong.material.ambient = Vector3(0.0f); phong.material.diffuse = Vector3(1.0f);
        phong.material.specular = Vector3(0.0f);
        bd.ByteWidth = sizeof(phong); initial.pSysMem = &phong;
        HR(device->CreateBuffer(&bd, &initial, &classicMaterial));
        D3D11_TEXTURE2D_DESC td{};
        td.Width = td.Height = td.MipLevels = td.ArraySize = td.SampleDesc.Count = 1;
        td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET;
        HR(device->CreateTexture2D(&td, nullptr, &output));
        HR(device->CreateRenderTargetView(output.Get(), nullptr, &target));
        td.BindFlags = 0; td.Usage = D3D11_USAGE_STAGING; td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        HR(device->CreateTexture2D(&td, nullptr, &staging));
        CreateShadow(false, twoD, twoDSrv);
        CreateShadow(true, cube, cubeSrv);
        GBufferValue(1, 1, 1, 1);       // albedo
        GBufferValue(.5f, .5f, 0, 1); // encoded -Z normal
        GBufferValue(0, .5f, 1, 1);   // metallic / roughness / AO
        GBufferValue(.5f, 0, 0, 0);   // world depth with identity inverse VP
        D3D11_SAMPLER_DESC sd{};
        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.MaxLOD = D3D11_FLOAT32_MAX;
        ComPtr<ID3D11SamplerState> sampler;
        HR(device->CreateSamplerState(&sd, &sampler));
        for (unsigned i = 0; i < 3; ++i) context->PSSetSamplers(i, 1, sampler.GetAddressOf());
        common.eyeWorld = Vector3(0, 0, -3);
        common.strengthIBL = 0; common.lightNum = 1;
        auto& light = common.light[0];
        light.strength = 3; light.direction = Vector3(0, 0, 1);
        light.position = Vector3(0, 0, -2); light.lightColor = Vector3(1);
        light.fallOffStart = 0; light.fallOffEnd = 20; light.spotPower = 1;
        light.shadowNear = .1f; light.shadowFar = 10; light.shadowFovY = 90;
        FillMaps(0); // stale occluding content already exists before the first frame
    }

    static void CheckLayout(ID3DBlob* code)
    {
        ComPtr<ID3D11ShaderReflection> reflection;
        HR(D3DReflect(code->GetBufferPointer(), code->GetBufferSize(), IID_PPV_ARGS(&reflection)));
        auto* cb = reflection->GetConstantBufferByName("CommonConstantBufferData");
        D3D11_SHADER_BUFFER_DESC desc{};
        HR(cb->GetDesc(&desc));
        Check(desc.Size == sizeof(CommonConstantBufferData), "CPU/HLSL constant size mismatch");
        const std::pair<const char*, size_t> fields[] = {
            {"lightNum", offsetof(CommonConstantBufferData, lightNum)},
            {"shadow2DMask", offsetof(CommonConstantBufferData, shadow2DMask)},
            {"shadowCubeMask", offsetof(CommonConstantBufferData, shadowCubeMask)},
            {"shadowPadding", offsetof(CommonConstantBufferData, shadowPadding)}
        };
        for (const auto& field : fields) {
            D3D11_SHADER_VARIABLE_DESC variable{};
            HR(cb->GetVariableByName(field.first)->GetDesc(&variable));
            Check(variable.StartOffset == field.second, "CPU/HLSL field offset mismatch");
        }
        std::cout << "LAYOUT size=" << desc.Size << " masks="
            << offsetof(CommonConstantBufferData, shadow2DMask) << ','
            << offsetof(CommonConstantBufferData, shadowCubeMask) << '\n';
    }

    ComPtr<ID3D11VertexShader> Vertex(const char* path)
    {
        std::wstring wide(path, path + std::strlen(path));
        auto code = Compile(wide.c_str(), "vs_5_0");
        ComPtr<ID3D11VertexShader> shader;
        HR(device->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &shader));
        return shader;
    }
    void CreateShadow(bool isCube, ComPtr<ID3D11Texture2D>& texture,
        ComPtr<ID3D11ShaderResourceView>& srv)
    {
        D3D11_TEXTURE2D_DESC td{};
        td.Width = td.Height = td.MipLevels = td.SampleDesc.Count = 1;
        td.ArraySize = MAX_LIGHTS * (isCube ? 6 : 1);
        td.Format = DXGI_FORMAT_R32_FLOAT; td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        td.MiscFlags = isCube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
        HR(device->CreateTexture2D(&td, nullptr, &texture));
        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = isCube ? D3D11_SRV_DIMENSION_TEXTURECUBEARRAY : D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        if (isCube) { sd.TextureCubeArray.MipLevels = 1; sd.TextureCubeArray.NumCubes = MAX_LIGHTS; }
        else { sd.Texture2DArray.MipLevels = 1; sd.Texture2DArray.ArraySize = MAX_LIGHTS; }
        HR(device->CreateShaderResourceView(texture.Get(), &sd, &srv));
    }
    void GBufferValue(float r, float g, float b, float a)
    {
        const float value[4] = {r, g, b, a};
        D3D11_TEXTURE2D_DESC td{};
        td.Width = td.Height = td.MipLevels = td.ArraySize = td.SampleDesc.Count = 1;
        td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA initial{value, sizeof(value), 0};
        ComPtr<ID3D11Texture2D> texture; ComPtr<ID3D11ShaderResourceView> srv;
        HR(device->CreateTexture2D(&td, &initial, &texture));
        HR(device->CreateShaderResourceView(texture.Get(), nullptr, &srv));
        gbuffer.push_back(srv);
    }
    void FillMaps(float depth)
    {
        ID3D11ShaderResourceView* empty[2]{}; context->PSSetShaderResources(15, 2, empty);
        for (unsigned i = 0; i < MAX_LIGHTS; ++i)
            context->UpdateSubresource(twoD.Get(), i, nullptr, &depth, sizeof(depth), 0);
        for (unsigned i = 0; i < MAX_LIGHTS * 6; ++i)
            context->UpdateSubresource(cube.Get(), i, nullptr, &depth, sizeof(depth), 0);
    }
    void Produce()
    {
        if (!writeMaps) return;
        FillMaps(nextDepth); ++writes;
        if (common.light[lightIndex].lightType == 2)
            for (unsigned face = 0; face < facesToWrite; ++face) BeginPointShadowFace(lightIndex, face);
        else BeginTwoDimensionalShadowPass(lightIndex);
        if (finishWrites) EndShadowPass();
    }
    // Same production state machine as the native Renderer, with no copied
    // availability rules. Real pipeline notifications drive the actual shaders.
    void BeginShadowFrame() override { shadowFrame.BeginFrame(); }
    void BeginShadowProduction() override { shadowFrame.BeginProduction(); }
    void EndShadowProduction(bool success) override { shadowFrame.EndProduction(success); }
    float Read(unsigned path)
    {
        const bool isDeferred = path == 1;
        common.shadow2DMask = shadowFrame.TwoDimensionalMask();
        common.shadowCubeMask = shadowFrame.PointMask();
        common.shadowPadding = 0;
        context->UpdateSubresource(constants.Get(), 0, nullptr, &common, 0, 0);
        context->PSSetConstantBuffers(1, 1, constants.GetAddressOf());
        context->PSSetConstantBuffers(2, 1, material.GetAddressOf());
        context->PSSetConstantBuffers(0, 1, classicMaterial.GetAddressOf());
        context->PSSetShaderResources(0, 1, gbuffer[0].GetAddressOf());
        // Intentionally keep stale maps bound: the consumer must reject them,
        // independently of a backend's defensive null-SRV binding.
        ID3D11ShaderResourceView* shadows[] = {twoDSrv.Get(), cubeSrv.Get()};
        context->PSSetShaderResources(15, 2, shadows);
        ID3D11ShaderResourceView* gb[] = {gbuffer[0].Get(), gbuffer[1].Get(), gbuffer[2].Get(), gbuffer[3].Get()};
        context->PSSetShaderResources(20, 4, gb);
        const float clear[4] = {-1, -1, -1, -1};
        context->ClearRenderTargetView(target.Get(), clear);
        context->OMSetRenderTargets(1, target.GetAddressOf(), nullptr);
        D3D11_VIEWPORT viewport{0, 0, 1, 1, 0, 1}; context->RSSetViewports(1, &viewport);
        context->IASetInputLayout(nullptr);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->VSSetShader(isDeferred ? deferredVs.Get() : forwardVs.Get(), nullptr, 0);
        context->PSSetShader(isDeferred ? deferred.Get() : (path == 2 ? classic.Get() : forward.Get()), nullptr, 0);
        context->Draw(3, 0);
        context->CopyResource(staging.Get(), output.Get());
        D3D11_MAPPED_SUBRESOURCE mapped{};
        HR(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped));
        float value = static_cast<float*>(mapped.pData)[0];
        context->Unmap(staging.Get(), 0);
        Check(std::isfinite(value) && value >= 0, "expected a finite rendered pixel");
        return value;
    }
    RenderPipeline& GetRenderPipeline() override { return pipeline; }
    void RenderFrame() override { pipeline.Execute(*this); }
    RenderTargetHandle CreateRenderTarget(const RenderTargetDesc&) override { return {}; }
    RenderTargetHandle FindRenderTarget(const std::string&) const override { return {}; }
    void ApplyRenderContext(const RenderContext&) override {}
    void BeginTwoDimensionalShadowPass(unsigned i) override { shadowFrame.RecordTwoDimensional(i); }
    void BeginPointShadowFace(unsigned i, unsigned face) override { shadowFrame.RecordPointFace(i, face); }
    void EndShadowPass() override { shadowFrame.SealWrites(); }
    void DrawMesh(ModelBuffer*, MeshDrawMode) override {}
    bool DrawMeshWithShaders(ModelBuffer*, const std::string&, const std::string&) override { return false; }
    void DrawShadowCaster(ModelBuffer*, ShadowCasterMode) override {}
    void DrawDeferredLighting() override {}
    void DrawParticles() override {}
    void DrawPostProcess() override {}
    void DrawLightGizmo(const Light&, bool, float) override {}
};

int main()
{
    int passed = 0, failed = 0;
    try {
        GpuRenderer renderer;
        auto test = [&](const std::string& name, const std::function<void()>& body) {
            try { body(); ++passed; std::cout << "PASS " << name << '\n'; }
            catch (const std::exception& e) { ++failed; std::cout << "FAIL " << name << ": " << e.what() << '\n'; }
        };
        for (unsigned path : {0u, 1u, 2u}) for (unsigned type : {1u, 2u, 3u}) {
            bool deferred = path == 1;
            std::string prefix = (deferred ? "Deferred" : (path == 2 ? "ClassicForward" : "Forward")) + std::to_string(type) + " ";
            auto& p = renderer.pipeline;
            p.Clear(); renderer.common.light[0].lightType = type;
            renderer.writeMaps = renderer.finishWrites = true; renderer.nextDepth = 0;
            bool predicate = true;
            RenderPassDesc desc; desc.name = "arbitrary producer name"; desc.type = RenderPassType::Shadow;
            desc.applyContext = false; desc.shouldExecute = [&] { return predicate; };
            desc.execute = [&](IDearsRenderer&) { renderer.Produce(); };
            IRenderPass* producer = p.AddPass(desc);
            float pixel = -1;
            RenderPassDesc consumer; consumer.name = "consumer";
            consumer.type = deferred ? RenderPassType::Lighting : RenderPassType::Forward;
            consumer.applyContext = false;
            consumer.execute = [&](IDearsRenderer&) { pixel = renderer.Read(path); };
            p.AddPass(consumer);
            producer->SetEnabled(false);
            test(prefix + "first-frame OFF keeps direct light", [&] { renderer.RenderFrame(); Check(pixel > .05f, "OFF still samples stale occlusion"); });
            std::cout << "PIXEL " << prefix << "first-OFF=" << pixel << '\n';
            producer->SetEnabled(true);
            test(prefix + "ON occluding map", [&] { renderer.RenderFrame(); Check(pixel < .001f, "fresh occlusion missing"); });
            std::cout << "PIXEL " << prefix << "ON-occluded=" << pixel << '\n';
            producer->SetEnabled(false);
            test(prefix + "direct SetEnabled OFF same frame", [&] { renderer.RenderFrame(); Check(pixel > .05f, "OFF suppressed direct light"); });
            renderer.nextDepth = 1; producer->SetEnabled(true);
            test(prefix + "back ON freshly lit map", [&] { renderer.RenderFrame(); Check(pixel > .05f, "ON reused old depth"); });
            std::cout << "PIXEL " << prefix << "back-ON-fresh-depth=" << pixel << '\n';
            renderer.nextDepth = 0; renderer.RenderFrame(); predicate = false;
            test(prefix + "predicate skipped", [&] { renderer.RenderFrame(); Check(pixel > .05f, "predicate skip reused old map"); });
            predicate = true; renderer.writeMaps = false;
            test(prefix + "no light production", [&] { renderer.RenderFrame(); Check(pixel > .05f, "empty producer reused old map"); });
            renderer.writeMaps = true; renderer.finishWrites = false;
            test(prefix + "incomplete producer", [&] { renderer.RenderFrame(); Check(pixel > .05f, "unsealed map became current"); });
            renderer.finishWrites = true; renderer.RenderFrame(); p.RemovePass(desc.name);
            test(prefix + "no shadow pass", [&] { renderer.RenderFrame(); Check(pixel > .05f, "removed pass reused old map"); });
            p.AddPass(desc);
            test(prefix + "consumer before producer", [&] { renderer.RenderFrame(); Check(pixel > .05f, "consumer saw prior frame map"); });
            p.MovePassBefore(desc.name, consumer.name);
            for (int i = 0; i < 3; ++i) {
                producer = p.FindPass(desc.name); producer->SetEnabled(false); renderer.RenderFrame();
                test(prefix + "repeated OFF " + std::to_string(i), [&] { Check(pixel > .05f, "repeated OFF reused map"); });
                producer->SetEnabled(true); renderer.RenderFrame();
                test(prefix + "repeated ON " + std::to_string(i), [&] { Check(pixel < .001f, "repeated ON missing occlusion"); });
            }
            if (type == 2) {
                renderer.facesToWrite = 5;
                test(prefix + "five cube faces are incomplete", [&] { renderer.RenderFrame(); Check(pixel > .05f, "partially stale cube became current"); });
                renderer.facesToWrite = 6;
            }
            RenderPassDesc toggle; toggle.name = "toggle between producer and consumer";
            toggle.applyContext = false;
            toggle.execute = [&](IDearsRenderer&) { producer->SetEnabled(false); };
            p.InsertPassBefore(consumer.name, toggle);
            test(prefix + "late direct disable", [&] { renderer.RenderFrame(); Check(pixel > .05f, "late disable kept generated maps current"); });
            p.RemovePass(toggle.name); producer->SetEnabled(true);
            RenderPassDesc second = desc; second.name = "second producer";
            second.shouldExecute = [] { return false; };
            p.InsertPassBefore(consumer.name, second);
            test(prefix + "skipped later producer supersedes maps", [&] { renderer.RenderFrame(); Check(pixel > .05f, "skipped latest producer reused maps"); });
            p.RemovePass(second.name);
            RenderPassDesc throwing = desc;
            throwing.execute = [&](IDearsRenderer&) { renderer.Produce(); throw std::runtime_error("producer failure"); };
            p.RemovePass(desc.name); p.InsertPassBefore(consumer.name, throwing);
            test(prefix + "exception invalidates sealed writes", [&] {
                try { renderer.RenderFrame(); } catch (const std::runtime_error&) {}
                Check(renderer.shadowFrame.TwoDimensionalMask() == 0 && renderer.shadowFrame.PointMask() == 0,
                    "throwing producer published maps");
                Check(renderer.Read(path) > .05f, "exception retained visible shadows");
            });
            p.Clear(); p.AddPass(desc); p.AddPass(consumer);
            Light configured = renderer.common.light[0];
            renderer.common.lightNum = MAX_LIGHTS;
            for (unsigned slot : {1u, 2u}) {
                for (auto& light : renderer.common.light) light.strength = 0;
                renderer.common.light[slot] = configured;
                renderer.lightIndex = slot;
                test(prefix + "per-light array slot " + std::to_string(slot), [&] {
                    renderer.RenderFrame(); Check(pixel < .001f, "wrong light map or mask bit");
                    p.FindPass(desc.name)->SetEnabled(false); renderer.RenderFrame();
                    Check(pixel > .05f, "per-light OFF suppressed ordinary light");
                    p.FindPass(desc.name)->SetEnabled(true);
                });
            }
            renderer.common.light[0] = configured;
            renderer.lightIndex = 0; renderer.common.lightNum = 1;
        }
        test("per-light masks, invalid indices, untyped writes and frame reset", [&] {
            ShadowFrameState<MAX_LIGHTS> state;
            state.RecordTwoDimensional(0); state.SealWrites(); state.EndProduction(true);
            Check(state.TwoDimensionalMask() == 0, "untyped writes became current");
            state.BeginProduction(); state.RecordTwoDimensional(1);
            state.RecordTwoDimensional(32); state.RecordPointFace(99, 0);
            for (unsigned face = 0; face < 6; ++face) state.RecordPointFace(2, face);
            state.RecordPointFace(0, 6); state.SealWrites();
            Check(state.TwoDimensionalMask() == 0 && state.PointMask() == 0, "published before callback returned");
            state.EndProduction(true);
            Check(state.TwoDimensionalMask() == 2 && state.PointMask() == 4, "wrong per-light masks");
            state.BeginFrame();
            Check(state.TwoDimensionalMask() == 0 && state.PointMask() == 0, "frame reused maps");
        });
    } catch (const std::exception& e) { std::cerr << "SETUP_FAILURE " << e.what() << '\n'; return 2; }
    std::cout << passed << " passed, " << failed << " failed\n";
    return failed ? 1 : 0;
}
