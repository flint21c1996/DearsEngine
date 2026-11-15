// RenderPipeline은 IDearsRenderer 인터페이스에만 의존하므로
// DirectX 디바이스나 창 없이 가짜 렌더러로 패스 구성/순서/컨텍스트 적용을 검증한다.
#include <windows.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "IDearsRenderer.h"
#include "RenderPipeline.h"
#include "RenderPreset.h"
#include "RenderRouting.h"
#include "SceneHistory.h"
#include "ShaderCatalog.h"

namespace {
int passed = 0;
int failed = 0;

void Require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

template<class Test> void Run(const std::string& name, Test test) {
    try {
        test();
        ++passed;
        std::cout << "PASS " << name << '\n';
    } catch (const std::exception& error) {
        ++failed;
        std::cout << "FAIL " << name << ": " << error.what() << '\n';
    }
}

// 호출된 순서를 문자열 로그로 남기는 가짜 렌더러.
class FakeRenderer final : public IDearsRenderer {
public:
    RenderPipeline pipeline;
    std::vector<std::string> log;
    std::vector<RenderContext> contexts;

    RenderPipeline& GetRenderPipeline() override { return pipeline; }
    void RenderFrame() override {
        log.push_back("begin");
        pipeline.Execute(*this);
        log.push_back("end");
    }
    RenderTargetHandle CreateRenderTarget(const RenderTargetDesc&) override { return RenderTargetHandle{ 1 }; }
    RenderTargetHandle FindRenderTarget(const std::string&) const override { return RenderTargetHandle(); }
    std::vector<RenderPassIO> appliedIO;
    void ApplyRenderContext(const RenderContext& renderContext) override {
        contexts.push_back(renderContext);
        // io 포인터는 호출 동안만 유효하므로 값으로 복사해 둔다.
        if (renderContext.io) appliedIO.push_back(*renderContext.io);
        log.push_back(std::string("apply:") + renderContext.GetPassName());
    }
    void BeginTwoDimensionalShadowPass(unsigned int) override { log.push_back("shadow2d"); }
    void BeginPointShadowFace(unsigned int, unsigned int) override { log.push_back("shadowCube"); }
    void EndShadowPass() override { log.push_back("shadowEnd"); }
    void DrawMesh(ModelBuffer*, MeshDrawMode) override { log.push_back("mesh"); }
    bool DrawMeshWithShaders(ModelBuffer*, const std::string& vs, const std::string& ps) override {
        log.push_back("shaders:" + vs + "+" + ps);
        return ShaderCatalog::AreCompatible(vs, ps);
    }
    void DrawShadowCaster(ModelBuffer*, ShadowCasterMode) override { log.push_back("caster"); }
    void DrawDeferredLighting() override { log.push_back("lighting"); }
    void DrawParticles() override { log.push_back("particles"); }
    void DrawPostProcess() override { log.push_back("post"); }
    void DrawLightGizmo(const Light&, bool, float) override { log.push_back("gizmo"); }
};

RenderPassDesc Logging(const std::string& name, std::vector<std::string>& log,
                       RenderPassType type = RenderPassType::Custom) {
    RenderPassDesc desc;
    desc.name = name;
    desc.type = type;
    desc.applyContext = false;
    desc.execute = [name, &log](IDearsRenderer&) { log.push_back(name); };
    return desc;
}

std::string Join(const std::vector<std::string>& values) {
    std::string joined;
    for (const std::string& value : values) {
        if (!joined.empty()) joined += ",";
        joined += value;
    }
    return joined;
}

void RequireOrder(const RenderPipeline& pipeline, const std::string& expected) {
    const std::string actual = Join(pipeline.GetPassNames());
    Require(actual == expected, "expected order [" + expected + "] but was [" + actual + "]");
}

class CountingPass final : public RenderPassBase {
public:
    explicit CountingPass(int& counter)
        : RenderPassBase("Counting", RenderPassType::Custom), m_counter(counter) {}
    void Execute(IDearsRenderer& renderer) override {
        ++m_counter;
        renderer.DrawParticles();
    }
private:
    int& m_counter;
};
}

int main() {
    Run("passes execute in registration order between frame begin and end", [] {
        FakeRenderer renderer;
        renderer.pipeline.AddPass(Logging("A", renderer.log));
        renderer.pipeline.AddPass(Logging("B", renderer.log));
        renderer.pipeline.AddPass(Logging("C", renderer.log));
        renderer.RenderFrame();
        Require(Join(renderer.log) == "begin,A,B,C,end", "got " + Join(renderer.log));
    });

    Run("context is applied before execute with the pass type, camera and buffer", [] {
        FakeRenderer renderer;
        Camera* camera = reinterpret_cast<Camera*>(static_cast<uintptr_t>(0x10));
        CommonConstantBufferData* buffer =
            reinterpret_cast<CommonConstantBufferData*>(static_cast<uintptr_t>(0x20));
        RenderPassDesc desc;
        desc.name = "Geometry";
        desc.type = RenderPassType::Geometry;
        desc.camera = camera;
        desc.commonBuffer = buffer;
        desc.execute = [&renderer](IDearsRenderer& r) {
            Require(&r == &renderer, "pass must receive the executing renderer");
            renderer.log.push_back("draw");
        };
        renderer.pipeline.AddPass(std::move(desc));
        renderer.pipeline.Execute(renderer);
        Require(Join(renderer.log) == "apply:GeometryPass,draw", "got " + Join(renderer.log));
        Require(renderer.contexts.size() == 1, "expected one context");
        Require(renderer.contexts[0].passType == RenderPassType::Geometry, "pass type mismatch");
        Require(renderer.contexts[0].camera == camera, "camera mismatch");
        Require(renderer.contexts[0].commonBuffer == buffer, "common buffer mismatch");
    });

    Run("applyContext=false leaves context setup to the pass", [] {
        FakeRenderer renderer;
        renderer.pipeline.AddPass(Logging("Shadow", renderer.log, RenderPassType::Shadow));
        renderer.pipeline.Execute(renderer);
        Require(renderer.contexts.empty(), "context must not be applied");
        Require(Join(renderer.log) == "Shadow", "got " + Join(renderer.log));
    });

    Run("shouldExecute=false skips both context and execute", [] {
        FakeRenderer renderer;
        bool hasShadowLight = false;
        RenderPassDesc desc = Logging("Shadow", renderer.log, RenderPassType::Shadow);
        desc.applyContext = true;
        desc.shouldExecute = [&hasShadowLight] { return hasShadowLight; };
        renderer.pipeline.AddPass(std::move(desc));
        renderer.pipeline.Execute(renderer);
        Require(renderer.log.empty(), "skipped pass must do nothing, got " + Join(renderer.log));
        hasShadowLight = true;
        renderer.pipeline.Execute(renderer);
        Require(Join(renderer.log) == "apply:ShadowPass,Shadow", "got " + Join(renderer.log));
    });

    Run("disabled passes are skipped and can be re-enabled", [] {
        FakeRenderer renderer;
        renderer.pipeline.AddPass(Logging("A", renderer.log));
        renderer.pipeline.AddPass(Logging("B", renderer.log));
        Require(renderer.pipeline.SetPassEnabled("A", false), "disable must succeed");
        Require(!renderer.pipeline.IsPassEnabled("A"), "A must report disabled");
        renderer.pipeline.Execute(renderer);
        Require(Join(renderer.log) == "B", "got " + Join(renderer.log));
        renderer.pipeline.SetPassEnabled("A", true);
        renderer.pipeline.Execute(renderer);
        Require(Join(renderer.log) == "B,A,B", "got " + Join(renderer.log));
        Require(!renderer.pipeline.SetPassEnabled("Missing", false), "unknown pass must fail");
    });

    Run("insert before/after places the pass relative to the anchor", [] {
        FakeRenderer renderer;
        renderer.pipeline.AddPass(Logging("Geometry", renderer.log));
        renderer.pipeline.AddPass(Logging("Forward", renderer.log));
        Require(renderer.pipeline.InsertPassAfter("Geometry", Logging("Lighting", renderer.log)) != nullptr,
                "insert after must succeed");
        Require(renderer.pipeline.InsertPassBefore("Geometry", Logging("Shadow", renderer.log)) != nullptr,
                "insert before must succeed");
        Require(renderer.pipeline.InsertPassAfter("Forward", Logging("Debug", renderer.log)) != nullptr,
                "insert after last must succeed");
        RequireOrder(renderer.pipeline, "Shadow,Geometry,Lighting,Forward,Debug");
        Require(renderer.pipeline.InsertPassAfter("Missing", Logging("X", renderer.log)) == nullptr,
                "missing anchor must fail");
        Require(!renderer.pipeline.HasPass("X"), "failed insert must not register");
    });

    Run("duplicate and empty names are rejected", [] {
        FakeRenderer renderer;
        Require(renderer.pipeline.AddPass(Logging("A", renderer.log)) != nullptr, "first add");
        Require(renderer.pipeline.AddPass(Logging("A", renderer.log)) == nullptr, "duplicate must fail");
        Require(renderer.pipeline.AddPass(Logging("", renderer.log)) == nullptr, "empty name must fail");
        Require(renderer.pipeline.AddPass(std::unique_ptr<IRenderPass>()) == nullptr, "null must fail");
        Require(renderer.pipeline.GetPassCount() == 1, "only one pass must remain");
    });

    Run("move and remove reorder the list", [] {
        FakeRenderer renderer;
        for (const char* name : { "A", "B", "C", "D" }) renderer.pipeline.AddPass(Logging(name, renderer.log));
        Require(renderer.pipeline.MovePassAfter("A", "C"), "move after");
        RequireOrder(renderer.pipeline, "B,C,A,D");
        Require(renderer.pipeline.MovePassBefore("D", "B"), "move before");
        RequireOrder(renderer.pipeline, "D,B,C,A");
        Require(!renderer.pipeline.MovePassBefore("A", "A"), "self move must fail");
        Require(!renderer.pipeline.MovePassBefore("A", "Missing"), "missing anchor must fail");
        RequireOrder(renderer.pipeline, "D,B,C,A");
        Require(renderer.pipeline.RemovePass("B"), "remove");
        Require(!renderer.pipeline.RemovePass("B"), "second remove must fail");
        RequireOrder(renderer.pipeline, "D,C,A");
        Require(renderer.pipeline.Clear(), "clear");
        Require(renderer.pipeline.GetPassCount() == 0, "empty after clear");
    });

    Run("structure cannot change while executing but enable toggles apply immediately", [] {
        FakeRenderer renderer;
        bool addRejected = false, removeRejected = false, clearRejected = false, moveRejected = false;
        RenderPassDesc first = Logging("First", renderer.log);
        first.execute = [&](IDearsRenderer& r) {
            RenderPipeline& pipeline = r.GetRenderPipeline();
            Require(pipeline.IsExecuting(), "must report executing");
            addRejected = pipeline.AddPass(Logging("Late", renderer.log)) == nullptr;
            removeRejected = !pipeline.RemovePass("Second");
            moveRejected = !pipeline.MovePassAfter("First", "Third");
            clearRejected = !pipeline.Clear();
            pipeline.SetPassEnabled("Second", false);
            renderer.log.push_back("First");
        };
        renderer.pipeline.AddPass(std::move(first));
        renderer.pipeline.AddPass(Logging("Second", renderer.log));
        renderer.pipeline.AddPass(Logging("Third", renderer.log));
        renderer.pipeline.Execute(renderer);
        Require(addRejected && removeRejected && moveRejected && clearRejected,
                "structural changes during Execute must be rejected");
        Require(Join(renderer.log) == "First,Third", "got " + Join(renderer.log));
        Require(!renderer.pipeline.IsExecuting(), "must not report executing afterwards");
        RequireOrder(renderer.pipeline, "First,Second,Third");
    });

    Run("re-entrant frame from inside a pass does not recurse", [] {
        FakeRenderer renderer;
        int executions = 0;
        RenderPassDesc desc = Logging("Recursive", renderer.log);
        desc.execute = [&executions](IDearsRenderer& r) {
            ++executions;
            r.GetRenderPipeline().Execute(r);
        };
        renderer.pipeline.AddPass(std::move(desc));
        renderer.pipeline.Execute(renderer);
        Require(executions == 1, "pass must run exactly once");
    });

    Run("an exception inside a pass does not leave the pipeline locked", [] {
        FakeRenderer renderer;
        RenderPassDesc desc = Logging("Throwing", renderer.log);
        desc.execute = [](IDearsRenderer&) { throw std::runtime_error("boom"); };
        renderer.pipeline.AddPass(std::move(desc));
        bool thrown = false;
        try { renderer.pipeline.Execute(renderer); } catch (const std::runtime_error&) { thrown = true; }
        Require(thrown, "exception must propagate");
        Require(!renderer.pipeline.IsExecuting(), "executing flag must be reset");
        Require(renderer.pipeline.RemovePass("Throwing"), "pipeline must be editable again");
    });

    Run("class based passes run through the same list", [] {
        FakeRenderer renderer;
        int counter = 0;
        IRenderPass* pass = renderer.pipeline.AddPass(std::make_unique<CountingPass>(counter));
        Require(pass != nullptr && pass == renderer.pipeline.FindPass("Counting"), "find must return the pass");
        renderer.pipeline.Execute(renderer);
        renderer.pipeline.Execute(renderer);
        Require(counter == 2, "class pass must run every frame");
        Require(Join(renderer.log) == "particles,particles", "got " + Join(renderer.log));
    });

    Run("a pass without explicit io passes a null io so the type default applies", [] {
        FakeRenderer renderer;
        RenderPassDesc desc;
        desc.name = "Forward";
        desc.type = RenderPassType::Forward;
        renderer.pipeline.AddPass(std::move(desc));
        renderer.pipeline.Execute(renderer);
        Require(renderer.contexts.size() == 1 && renderer.contexts[0].passType == RenderPassType::Forward,
                "context must be applied");
        Require(renderer.appliedIO.empty(), "io must be null when not specified");
    });

    Run("explicit io reaches the renderer unchanged", [] {
        FakeRenderer renderer;
        const RenderTargetHandle minimap = renderer.CreateRenderTarget(RenderTargetDesc{});
        RenderPassIO io;
        io.color = PassColorOutput::Offscreen;
        io.offscreenTarget = minimap;
        io.depth = PassDepthOutput::None;
        io.clearColor = true;
        io.clearColorValue[2] = 0.5f;
        io.inputs.push_back(RenderTargetInput{ RenderTargetHandle{ 7 }, 30 });
        RenderPassDesc desc;
        desc.name = "Minimap";
        desc.io = io;
        renderer.pipeline.AddPass(std::move(desc));
        renderer.pipeline.Execute(renderer);
        Require(renderer.appliedIO.size() == 1, "io must be forwarded");
        const RenderPassIO& applied = renderer.appliedIO[0];
        Require(applied.color == PassColorOutput::Offscreen && applied.offscreenTarget == minimap,
                "output mismatch");
        Require(applied.depth == PassDepthOutput::None && applied.clearColor && !applied.clearDepth,
                "depth/clear mismatch");
        Require(applied.clearColorValue[2] == 0.5f, "clear color mismatch");
        Require(applied.inputs.size() == 1 && applied.inputs[0].target.id == 7 &&
                applied.inputs[0].shaderSlot == 30, "inputs mismatch");
    });

    Run("type defaults reproduce the deferred frame bindings", [] {
        RenderPassIO io;
        Require(GetDefaultRenderPassIO(RenderPassType::Geometry, io) &&
                io.color == PassColorOutput::GBuffer && !io.readGBuffer, "geometry default");
        Require(GetDefaultRenderPassIO(RenderPassType::Lighting, io) &&
                io.color == PassColorOutput::BackBuffer && io.depth == PassDepthOutput::Default &&
                io.readGBuffer, "lighting default");
        Require(GetDefaultRenderPassIO(RenderPassType::Forward, io) &&
                io.color == PassColorOutput::BackBuffer && io.depth == PassDepthOutput::GBufferDepth &&
                !io.readGBuffer, "forward default");
        for (RenderPassType type : { RenderPassType::Unknown, RenderPassType::Shadow, RenderPassType::Particle,
                                     RenderPassType::PostProcess, RenderPassType::SelectionOutline,
                                     RenderPassType::Debug, RenderPassType::Ui, RenderPassType::Custom }) {
            Require(!GetDefaultRenderPassIO(type, io), "types without a default must keep previous bindings");
        }
    });

    Run("normalize repairs combinations the graphics API rejects", [] {
        RenderPassIO io;
        io.color = PassColorOutput::Offscreen;
        Require(NormalizeRenderPassIO(io).color == PassColorOutput::Keep,
                "offscreen without a handle must keep the previous output");

        io = RenderPassIO();
        io.color = PassColorOutput::GBuffer;
        io.readGBuffer = true;
        io.depth = PassDepthOutput::None;
        RenderPassIO fixed = NormalizeRenderPassIO(io);
        Require(!fixed.readGBuffer && fixed.depth == PassDepthOutput::Default,
                "cannot read the G-Buffer while writing it");

        io = RenderPassIO();
        io.readGBuffer = true;
        fixed = NormalizeRenderPassIO(io);
        Require(fixed.color == PassColorOutput::BackBuffer, "reading the G-Buffer needs an explicit output");

        io.color = PassColorOutput::BackBuffer;
        io.depth = PassDepthOutput::GBufferDepth;
        fixed = NormalizeRenderPassIO(io);
        Require(fixed.depth == PassDepthOutput::None && fixed.readGBuffer,
                "G-Buffer depth cannot be read and bound as depth at once");

        io = RenderPassIO();
        io.color = PassColorOutput::BackBuffer;
        io.depth = PassDepthOutput::GBufferDepth;
        fixed = NormalizeRenderPassIO(io);
        Require(fixed.color == PassColorOutput::BackBuffer && fixed.depth == PassDepthOutput::GBufferDepth,
                "valid combinations must pass through unchanged");
    });

    Run("objects without overrides keep the route they had before shaders were selectable", [] {
        using enum SceneRenderType;
        const auto route = [](SceneRenderType type, SceneRenderPath path, MaterialShadingModel model) {
            return ResolveMeshRoute(type, path, model, "", "");
        };
        // 예전 규칙: Deferred로 지정한 Default Lit PBR 메시만 G-Buffer로 간다.
        MeshRoute pbr = route(PbrMesh, SceneRenderPath::Deferred, MaterialShadingModel::DefaultLit);
        Require(pbr.deferred && !pbr.useNamedShaders, "deferred default-lit PBR must stay deferred");
        Require(pbr.vertexShader == "Pbr" && pbr.pixelShader == "GBuffer", "deferred PBR shaders");
        Require(!route(PbrMesh, SceneRenderPath::Forward, MaterialShadingModel::DefaultLit).deferred,
                "forward PBR must stay forward");
        Require(!route(PbrMesh, SceneRenderPath::Deferred, MaterialShadingModel::ThinFilm).deferred,
                "thin film must stay forward");
        Require(route(PbrMesh, SceneRenderPath::Forward, MaterialShadingModel::ThinFilm).pixelShader == "ThinFilm",
                "thin film default pixel shader");
        for (SceneRenderType type : { StaticMesh, SkinnedMesh, EquipmentMesh, Billboard }) {
            const MeshRoute r = route(type, SceneRenderPath::Deferred, MaterialShadingModel::DefaultLit);
            Require(r.isConfigurable && !r.deferred && !r.useNamedShaders,
                    "non-PBR meshes must stay forward on the default path");
        }
        for (SceneRenderType type : { CubeMap, DirectionalLight, PointLight, SpotLight }) {
            const MeshRoute r = route(type, SceneRenderPath::Deferred, MaterialShadingModel::DefaultLit);
            Require(!r.isConfigurable && !r.deferred && !r.useNamedShaders, "fixed-path types are not configurable");
        }
    });

    Run("shader overrides choose the named path and stay compatible", [] {
        // Static 메시에 Unlit PS만 바꾸기.
        MeshRoute r = ResolveMeshRoute(SceneRenderType::StaticMesh, SceneRenderPath::Forward,
                                       MaterialShadingModel::DefaultLit, "", "Unlit");
        Require(r.useNamedShaders && r.vertexShader == "Static" && r.pixelShader == "Unlit", "ps override");

        // Static 메시에 PBR VS를 고르면 PS 기본값도 같은 계열로 따라간다.
        r = ResolveMeshRoute(SceneRenderType::StaticMesh, SceneRenderPath::Forward,
                             MaterialShadingModel::DefaultLit, "Pbr", "");
        Require(r.useNamedShaders && r.vertexShader == "Pbr" && r.pixelShader == "PbrLit", "vs override");
        Require(ShaderCatalog::AreCompatible(r.vertexShader, r.pixelShader), "must be compatible");

        // 계열이 다른 조합은 VS 계열의 기본 PS로 고친다.
        r = ResolveMeshRoute(SceneRenderType::PbrMesh, SceneRenderPath::Forward,
                             MaterialShadingModel::DefaultLit, "Skinned", "ThinFilm");
        Require(r.vertexShader == "Skinned" && r.pixelShader == "Lit", "mismatched pair must be repaired");

        // PBR VS로 바꾼 Static 메시는 Deferred로 보낼 수 있고 PS는 G-Buffer로 고정된다.
        r = ResolveMeshRoute(SceneRenderType::StaticMesh, SceneRenderPath::Deferred,
                             MaterialShadingModel::DefaultLit, "Pbr", "PbrLit");
        Require(r.deferred && r.useNamedShaders && r.pixelShader == "GBuffer", "override into deferred");

        // PBR 메시를 일반 계열 VS로 바꾸면 Deferred를 골라도 Forward로 그린다.
        r = ResolveMeshRoute(SceneRenderType::PbrMesh, SceneRenderPath::Deferred,
                             MaterialShadingModel::DefaultLit, "Static", "");
        Require(!r.deferred && r.pixelShader == "Lit", "basic linkage cannot be deferred");

        // Thin Film을 직접 고르면 Forward 전용이다.
        r = ResolveMeshRoute(SceneRenderType::PbrMesh, SceneRenderPath::Deferred,
                             MaterialShadingModel::DefaultLit, "", "ThinFilm");
        Require(!r.deferred && r.pixelShader == "ThinFilm", "thin film override is forward only");

        // 모르는 이름과 G-Buffer 직접 지정은 재정의로 취급하지 않는다.
        r = ResolveMeshRoute(SceneRenderType::PbrMesh, SceneRenderPath::Forward,
                             MaterialShadingModel::DefaultLit, "NoSuchShader", "GBuffer");
        Require(!r.useNamedShaders && r.vertexShader == "Pbr" && r.pixelShader == "PbrLit",
                "invalid overrides must fall back to defaults");
    });

    Run("every selectable pair in the catalog resolves to a compatible route", [] {
        for (const ShaderInfo& vs : ShaderCatalog::VertexShaders()) {
            for (const ShaderInfo& ps : ShaderCatalog::PixelShaders()) {
                for (SceneRenderPath path : { SceneRenderPath::Forward, SceneRenderPath::Deferred }) {
                    const MeshRoute r = ResolveMeshRoute(SceneRenderType::StaticMesh, path,
                                                         MaterialShadingModel::DefaultLit, vs.name, ps.name);
                    Require(ShaderCatalog::AreCompatible(r.vertexShader, r.pixelShader),
                            std::string("incompatible result for ") + vs.name + "+" + ps.name);
                    Require(r.deferred == (r.pixelShader == "GBuffer"),
                            "G-Buffer pixel shader must be used exactly when deferred");
                }
            }
        }
        Require(!ShaderCatalog::AreCompatible("Static", "PbrLit"), "cross linkage must be incompatible");
        Require(!ShaderCatalog::AreCompatible("Missing", "Lit"), "unknown shader must be incompatible");
    });

    Run("shadow caster follows the vertex shader override", [] {
        Require(ResolveShadowCasterMode(SceneRenderType::PbrMesh, "") == ShadowCasterMode::Pbr, "pbr default");
        Require(ResolveShadowCasterMode(SceneRenderType::SkinnedMesh, "") == ShadowCasterMode::Skinned, "skinned default");
        Require(ResolveShadowCasterMode(SceneRenderType::EquipmentMesh, "") == ShadowCasterMode::Equipment, "equipment default");
        Require(ResolveShadowCasterMode(SceneRenderType::StaticMesh, "") == ShadowCasterMode::Static, "static default");
        Require(ResolveShadowCasterMode(SceneRenderType::PbrMesh, "Static") == ShadowCasterMode::Static, "override to static");
        Require(ResolveShadowCasterMode(SceneRenderType::StaticMesh, "Pbr") == ShadowCasterMode::Pbr, "override to pbr");
        Require(ResolveShadowCasterMode(SceneRenderType::StaticMesh, "Water") == ShadowCasterMode::Static, "water casts as static");
        Require(ResolveShadowCasterMode(SceneRenderType::SkinnedMesh, "Unknown") == ShadowCasterMode::Skinned, "unknown falls back");
    });

    Run("preset library keeps names unique and repairs invalid shader pairs", [] {
        RenderPresetLibrary library;
        Require(library.Add({ "Glass", "Pbr", "ThinFilm", SceneRenderPath::Forward }) == "Glass", "first name");
        Require(library.Add({ "Glass" }) == "Glass 2", "duplicate name gets a number");
        Require(library.Add({ "" }) == "New Preset", "empty name gets a default");

        // 계열이 다른 조합, 없는 셰이더, G-Buffer 직접 지정은 Auto로 고쳐진다.
        library.Add({ "Mismatch", "Static", "PbrLit", SceneRenderPath::Forward });
        Require(library.Find("Mismatch")->vertexShader == "Static" && library.Find("Mismatch")->pixelShader.empty(),
                "mismatched pixel shader must become Auto");
        library.Add({ "Unknown", "NoSuchVS", "GBuffer", SceneRenderPath::Deferred });
        Require(library.Find("Unknown")->vertexShader.empty() && library.Find("Unknown")->pixelShader.empty(),
                "unknown and G-Buffer shaders must become Auto");
        // Vertex Shader가 Auto면 Pixel Shader는 그대로 둔다. (오브젝트마다 계열이 다를 수 있다.)
        library.Add({ "AutoUnlit", "", "Unlit", SceneRenderPath::Forward });
        Require(library.Find("AutoUnlit")->pixelShader == "Unlit", "Auto vertex shader keeps the pixel shader");
    });

    Run("preset library update, rename and remove", [] {
        RenderPresetLibrary library;
        library.Add({ "A", "Pbr", "PbrLit", SceneRenderPath::Forward });
        library.Add({ "B" });

        Require(library.Update("A", { "ignored", "Pbr", "", SceneRenderPath::Deferred }), "update");
        const RenderPreset* a = library.Find("A");
        Require(a && a->pixelShader.empty() && a->renderPath == SceneRenderPath::Deferred, "update keeps the name");
        Require(!library.Update("Missing", {}), "update of a missing preset must fail");

        Require(!library.Rename("A", "B"), "rename onto an existing name must fail");
        Require(!library.Rename("A", ""), "rename to an empty name must fail");
        Require(library.Rename("A", "C") && !library.Find("A") && library.Find("C"), "rename");
        Require(library.Find("C")->renderPath == SceneRenderPath::Deferred, "rename keeps the contents");

        Require(library.Remove("B") && !library.Remove("B"), "remove once");
        Require(library.GetPresets().size() == 1, "one preset left");
    });

    Run("preset library round-trips through its file", [] {
        const std::string path = "render-presets-roundtrip.txt";
        RenderPresetLibrary saved;
        saved.AddDefaults();
        saved.Add({ "My \"Quoted\" Preset", "Skinned", "Unlit", SceneRenderPath::Forward });
        Require(saved.Save(path), "save");

        RenderPresetLibrary loaded;
        loaded.Add({ "Stale" });
        Require(loaded.Load(path), "load");
        Require(loaded.GetPresets().size() == saved.GetPresets().size(), "count mismatch");
        Require(!loaded.Find("Stale"), "load must replace the previous contents");
        for (const RenderPreset& expected : saved.GetPresets()) {
            const RenderPreset* actual = loaded.Find(expected.name);
            Require(actual != nullptr, "missing preset " + expected.name);
            Require(actual->vertexShader == expected.vertexShader && actual->pixelShader == expected.pixelShader &&
                    actual->renderPath == expected.renderPath, "contents mismatch for " + expected.name);
        }

        RenderPresetLibrary untouched;
        untouched.Add({ "Keep" });
        Require(!untouched.Load("no-such-render-presets.txt"), "missing file must fail");
        Require(untouched.Find("Keep") != nullptr, "failed load must not clear the library");
    });

    Run("default presets produce the routes their names promise", [] {
        RenderPresetLibrary library;
        library.AddDefaults();
        const auto route = [&library](const char* presetName, SceneRenderType type) {
            const RenderPreset* preset = library.Find(presetName);
            Require(preset != nullptr, std::string("missing default preset ") + presetName);
            return ResolveMeshRoute(type, preset->renderPath, MaterialShadingModel::DefaultLit,
                                    preset->vertexShader, preset->pixelShader);
        };

        // Vertex Shader가 Auto인 프리셋은 메시 종류를 따라간다.
        MeshRoute r = route("Unlit", SceneRenderType::SkinnedMesh);
        Require(r.vertexShader == "Skinned" && r.pixelShader == "Unlit" && !r.deferred, "Unlit on skinned");
        r = route("Unlit", SceneRenderType::StaticMesh);
        Require(r.vertexShader == "Static" && r.pixelShader == "Unlit", "Unlit on static");

        r = route("PBR Deferred", SceneRenderType::PbrMesh);
        Require(r.deferred && r.vertexShader == "Pbr" && r.pixelShader == "GBuffer", "PBR Deferred");
        r = route("PBR Forward", SceneRenderType::PbrMesh);
        Require(!r.deferred && r.pixelShader == "PbrLit", "PBR Forward");
        r = route("Thin Film", SceneRenderType::PbrMesh);
        Require(!r.deferred && r.pixelShader == "ThinFilm", "Thin Film");

        // 일반 계열 Pixel Shader 프리셋을 PBR 메시에 놓으면 그 메시 계열의 기본값으로 그린다.
        r = route("Lit", SceneRenderType::PbrMesh);
        Require(r.vertexShader == "Pbr" && r.pixelShader == "PbrLit", "Lit on a PBR mesh falls back");
    });

    Run("scene history undoes and redoes committed states", [] {
        SceneHistory history;
        history.Reset("empty");
        Require(!history.CanUndo() && !history.CanRedo(), "fresh history has nothing to undo");
        Require(history.Commit("moved"), "first edit");
        Require(history.Commit("deleted"), "second edit");
        Require(!history.Commit("deleted"), "an unchanged scene must not add a step");
        Require(history.GetUndoCount() == 2 && history.GetRedoCount() == 0, "two steps recorded");

        const std::string* state = history.Undo();
        Require(state && *state == "moved", "undo returns the previous state");
        state = history.Undo();
        Require(state && *state == "empty", "undo reaches the loaded scene");
        Require(history.Undo() == nullptr, "cannot undo past the loaded scene");

        state = history.Redo();
        Require(state && *state == "moved", "redo walks forward again");
        state = history.Redo();
        Require(state && *state == "deleted" && history.Redo() == nullptr, "redo stops at the newest state");
    });

    Run("a new edit after undo discards the redo branch", [] {
        SceneHistory history;
        history.Reset("a");
        history.Commit("b");
        history.Commit("c");
        history.Undo();
        Require(history.CanRedo(), "redo is available right after undo");
        Require(history.Commit("d"), "new edit");
        Require(!history.CanRedo(), "redo branch must be gone");
        const std::string* state = history.Undo();
        Require(state && *state == "b", "undo goes to the state before the new edit, not the discarded one");
    });

    Run("scene history drops the oldest states at capacity and resets cleanly", [] {
        SceneHistory history(3);
        history.Reset("s0");
        for (const char* state : { "s1", "s2", "s3", "s4" }) history.Commit(state);
        Require(history.GetUndoCount() == 2, "capacity 3 keeps the current state plus two undo steps");
        Require(*history.Undo() == "s3" && *history.Undo() == "s2" && history.Undo() == nullptr,
                "oldest states are dropped first");

        history.Reset("other scene");
        Require(!history.CanUndo() && !history.CanRedo(), "opening another scene starts a new history");

        SceneHistory unprimed;
        Require(unprimed.Undo() == nullptr && unprimed.Redo() == nullptr, "empty history is safe");
        Require(unprimed.Commit("first") && !unprimed.CanUndo(), "first commit becomes the base state");
    });

    std::cout << "PASSED=" << passed << " FAILED=" << failed << '\n';
    return failed == 0 ? 0 : 1;
}
