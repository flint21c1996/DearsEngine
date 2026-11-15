#include "DemoScene.h"
#include <imgui_internal.h>

#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

// Explicit template instantiation permits naming private members (C++ access
// rules). This test peer calls the real linked DemoScene, without changing
// production access, redefining 'private', extracting bodies or mocking parsing.
template<class Tag, typename Tag::type Member> struct SceneAccess
{
    friend typename Tag::type Get(Tag) { return Member; }
};
using NameFunction = void (DemoScene::*)(const std::string&);
using ReadFunction = bool (DemoScene::*)(std::istream&);
using RestoreFunction = bool (DemoScene::*)(const std::string&);
using VoidFunction = void (DemoScene::*)();
using SerializeFunction = std::string (DemoScene::*)() const;
using SaveFunction = void (DemoScene::*)() const;
using PlayFunction = void (DemoScene::*)(bool);
using ObjectMember = std::vector<std::unique_ptr<RenderObject>> DemoScene::*;
using NameMember = std::vector<std::string> DemoScene::*;
using DescMember = std::vector<SceneObjectCreateDesc> DemoScene::*;
using PathMember = std::string DemoScene::*;
using SelectionMember = int DemoScene::*;
using HistoryMember = SceneHistory DemoScene::*;
using DirtyMember = bool DemoScene::*;
using CreateFunction = void (DemoScene::*)(const SceneObjectCreateDesc&, Matrix, Matrix, Matrix, bool);
#define SCENE_ACCESS(Tag, Type, Member) \
    struct Tag { using type = Type; friend type Get(Tag); }; \
    template struct SceneAccess<Tag, &DemoScene::Member>
SCENE_ACCESS(New, NameFunction, NewScene);
SCENE_ACCESS(Open, NameFunction, OpenScene);
SCENE_ACCESS(Read, ReadFunction, ReadScene);
SCENE_ACCESS(Restore, RestoreFunction, RestoreScene);
SCENE_ACCESS(Serialize, SerializeFunction, SerializeScene);
SCENE_ACCESS(Save, SaveFunction, SaveSceneToFile);
SCENE_ACCESS(Undo, VoidFunction, Undo);
SCENE_ACCESS(Redo, VoidFunction, Redo);
SCENE_ACCESS(Play, PlayFunction, SetPlaying);
SCENE_ACCESS(Popups, VoidFunction, DrawScenePopups);
SCENE_ACCESS(Objects, ObjectMember, m_objects);
SCENE_ACCESS(Names, NameMember, m_objectNames);
SCENE_ACCESS(Descs, DescMember, m_objectCreateDescs);
SCENE_ACCESS(Path, PathMember, m_scenePath);
SCENE_ACCESS(Selected, SelectionMember, m_selectedObjectIndex);
SCENE_ACCESS(History, HistoryMember, m_history);
SCENE_ACCESS(Dirty, DirtyMember, m_historyDirty);
SCENE_ACCESS(PlaySnapshot, PathMember, m_playSnapshot);
SCENE_ACCESS(Create, CreateFunction, CreateObjectFromDesc);
#undef SCENE_ACCESS

static void Check(bool value, const char* message)
{
    if (!value) throw std::runtime_error(message);
}
static std::string Text(DemoScene& scene) { return (scene.*Get(Serialize{}))(); }
static std::string File(const fs::path& path)
{
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file), {}};
}
static std::string FileText(const fs::path& path)
{
    std::ifstream file(path); // same Windows text newline handling as production
    return {std::istreambuf_iterator<char>(file), {}};
}
static void Write(const fs::path& path, const std::string& contents)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << contents;
    Check(bool(file), "fixture write failed");
}
static void AddLight(DemoScene& scene, const std::string& name)
{
    SceneObjectCreateDesc desc;
    desc.name = name; desc.vertexBufferName.clear(); desc.textureName.clear();
    desc.renderType = SceneRenderType::DirectionalLight;
    (scene.*Get(Create{}))(desc, Matrix(), Matrix(), Matrix(), false);
}
static std::unique_ptr<DemoScene> Seed()
{
    auto scene = std::make_unique<DemoScene>(nullptr);
    AddLight(*scene, "current one"); AddLight(*scene, "current two");
    scene.get()->*Get(Path{}) = "../Scenes/current.txt";
    scene.get()->*Get(Selected{}) = 1;
    auto& history = scene.get()->*Get(History{});
    history.Reset(Text(*scene));
    (scene.get()->*Get(Objects{}))[0]->ObjectPos = Matrix::CreateTranslation(2, 0, 0);
    history.Commit(Text(*scene));
    (scene.get()->*Get(Objects{}))[0]->ObjectPos = Matrix::CreateTranslation(4, 0, 0);
    history.Commit(Text(*scene)); history.Undo();
    scene.get()->*Get(Dirty{}) = true;
    // Real list storage/pointers are seeded too: failed loads must not clear or
    // replace any list, even if the currently selected headless fixtures are lights.
    auto* object = (scene.get()->*Get(Objects{}))[0].get();
    const_cast<std::vector<SceneRenderItem>&>(scene->GetMainRenderItems()).push_back({object});
    const_cast<std::vector<SceneRenderItem>&>(scene->GetShadowRenderItems()).push_back({object});
    Write("../Scenes/current.txt", Text(*scene));
    return scene;
}
static std::string HistoryText(SceneHistory history)
{
    std::ostringstream text;
    text << history.GetUndoCount() << ':' << history.GetRedoCount() << ':';
    SceneHistory current = history;
    current.Commit("test-only history fingerprint marker");
    if (const auto* state = current.Undo()) text << std::quoted(*state);
    while (const auto* state = history.Undo()) text << std::quoted(*state);
    while (const auto* state = history.Redo()) text << std::quoted(*state);
    return text.str();
}
struct Snapshot
{
    std::string text, path, history, disk;
    fs::path diskPath;
    const SceneObjectCreateDesc* descriptors;
    size_t descriptorCount;
    std::vector<RenderObject*> objects;
    std::vector<std::string> names;
    std::vector<SceneRenderItem> main, shadow;
    int selection;
    bool dirty;
    explicit Snapshot(DemoScene& scene)
        : text(Text(scene)), path(scene.*Get(Path{})), history(HistoryText(scene.*Get(History{}))),
          names(scene.*Get(Names{})), main(scene.GetMainRenderItems()), shadow(scene.GetShadowRenderItems()),
          selection(scene.*Get(Selected{})), dirty(scene.*Get(Dirty{}))
    {
        for (const auto& object : scene.*Get(Objects{})) objects.push_back(object.get());
        descriptors = (scene.*Get(Descs{})).data();
        descriptorCount = (scene.*Get(Descs{})).size();
        diskPath = fs::absolute(path); disk = File(diskPath);
    }
    void Unchanged(DemoScene& scene) const
    {
        Check(Text(scene) == text, "live scene content changed");
        Check(scene.*Get(Path{}) == path, "current path changed");
        Check(HistoryText(scene.*Get(History{})) == history, "Undo/Redo history changed");
        Check(scene.*Get(Selected{}) == selection && scene.*Get(Dirty{}) == dirty, "selection/dirty state changed");
        Check(scene.*Get(Names{}) == names, "object names changed");
        Check((scene.*Get(Descs{})).data() == descriptors && (scene.*Get(Descs{})).size() == descriptorCount,
            "live descriptor storage replaced");
        Check(File(diskPath) == disk, "current scene file changed on failure");
        Check((scene.*Get(Objects{})).size() == objects.size(), "object count changed");
        for (size_t i = 0; i < objects.size(); ++i)
            Check((scene.*Get(Objects{}))[i].get() == objects[i], "live object identity replaced");
        auto same = [](const auto& a, const auto& b) {
            if (a.size() != b.size()) return false;
            for (size_t i = 0; i < a.size(); ++i)
                if (a[i].object != b[i].object || a[i].renderType != b[i].renderType || a[i].renderPath != b[i].renderPath) return false;
            return true;
        };
        Check(same(main, scene.GetMainRenderItems()) && same(shadow, scene.GetShadowRenderItems()), "render lists changed");
    }
};

// Explicit supported-file fixtures, not a second parser. GPU-free light records
// still use real RenderObject initialization and real production object creation.
static std::string Record(int version, const std::string& name)
{
    std::ostringstream out;
    out << std::quoted(name);
    for (int i = 0; i < 10; ++i) out << " \"\"";
    out << " DirectionalLight";
    if (version >= 2) out << " Deferred";
    if (version >= 6) out << " DefaultLit";
    if (version >= 7) out << " \"\" \"\"";
    if (version >= 8) out << " \"\"";
    out << " 1";
    if (version >= 3) out << " 0.2 0.4 0.1";
    if (version >= 6) out << " 1 1.33 0 100";
    if (version >= 4) out << " 10 20 30 2 1 0.5 0.25 20 32";
    if (version >= 5) out << " 0.1 100 70 30";
    for (int matrix = 0; matrix < 3; ++matrix)
        for (int i = 0; i < 16; ++i) out << ' ' << (i % 5 == 0 ? 1 : 0);
    return out.str() + '\n';
}
static std::string Scene(int version, unsigned count = 2)
{
    std::string result = "DearsScene " + std::to_string(version) + "\n" + std::to_string(count) + "\n";
    for (unsigned i = 0; i < count; ++i) result += Record(version, "loaded " + std::to_string(i));
    return result;
}
static std::string Changed(std::string text, const std::string& before, const std::string& after)
{
    auto at = text.find(before); Check(at != std::string::npos, "fixture token missing");
    text.replace(at, before.size(), after); return text;
}
static bool ErrorPopup(DemoScene& scene)
{
    // Fresh real ImGui state prevents a prior case's open modal causing a false pass.
    ImGui::DestroyContext(); ImGui::CreateContext();
    auto& io = ImGui::GetIO(); io.IniFilename = nullptr; io.DisplaySize = ImVec2(1280, 720);
    unsigned char* pixels; int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    ImGui::NewFrame(); ImGui::Begin("scene-test-host");
    (scene.*Get(Popups{}))();
    auto* popup = ImGui::FindWindowByName("Scene File Error");
    bool visible = popup && popup->Active;
    ImGui::End(); ImGui::Render(); return visible;
}

int main()
{
    int passed = 0, failed = 0;
    try {
        wchar_t module[MAX_PATH]{}; GetModuleFileNameW(nullptr, module, MAX_PATH);
        const fs::path allowed = fs::path(module).parent_path().parent_path() / "sandbox" / "Demo";
        Check(fs::equivalent(fs::current_path(), allowed), "refusing file tests outside ignored task sandbox");
        fs::create_directories("../Scenes");
        ImGui::CreateContext();
        auto& io = ImGui::GetIO(); io.IniFilename = nullptr; io.DisplaySize = ImVec2(1280, 720);
        unsigned char* pixels; int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        auto test = [&](const std::string& name, const std::function<void()>& body) {
            try { body(); ++passed; std::cout << "PASS " << name << '\n'; }
            catch (const std::exception& error) { ++failed; std::cout << "FAIL " << name << ": " << error.what() << '\n'; }
        };
        test("New existing name preserves original disk bytes", [] {
            auto scene = Seed(); const std::string original = Scene(8);
            Write("../Scenes/existing.txt", original);
            (scene.get()->*Get(New{}))("existing");
            Check(File("../Scenes/existing.txt") == original, "existing file was overwritten");
        });
        test("New collision preserves all live state path and history", [] {
            auto scene = Seed(); Write("../Scenes/existing.txt", Scene(8));
            Snapshot before(*scene); (scene.get()->*Get(New{}))("existing"); before.Unchanged(*scene);
        });
        test("New case-insensitive Windows collision preserves disk and live state", [] {
            auto scene = Seed(); const auto original = Scene(8); Write("../Scenes/casecollision.txt", original);
            Snapshot before(*scene); (scene.get()->*Get(New{}))("CASECOLLISION"); before.Unchanged(*scene);
            Check(File("../Scenes/casecollision.txt") == original, "Windows name collision overwrote data");
        });
        test("New unavailable destination preserves current scene", [] {
            auto scene = Seed(); fs::create_directories("../Scenes/directory.txt");
            Snapshot before(*scene); (scene.get()->*Get(New{}))("directory"); before.Unchanged(*scene);
        });
        test("New directory creation failure preserves current state", [] {
            auto scene = Seed(); Snapshot before(*scene);
            const auto root = fs::current_path().parent_path() / "failed-directory";
            fs::create_directories(root / "Demo"); Write(root / "Scenes", "not a directory");
            struct DirectoryGuard {
                fs::path previous = fs::current_path();
                ~DirectoryGuard() { fs::current_path(previous); }
            } guard;
            fs::current_path(root / "Demo");
            (scene.get()->*Get(New{}))("must-not-commit"); before.Unchanged(*scene);
        });
        test("New unique scene succeeds with empty content and fresh history", [] {
            fs::remove("../Scenes/fresh.txt"); auto scene = Seed();
            (scene.get()->*Get(New{}))("fresh");
            Check((scene.get()->*Get(Objects{})).empty(), "New did not clear objects");
            Check(scene.get()->*Get(Path{}) == "../Scenes/fresh.txt", "New path incorrect");
            Check(FileText("../Scenes/fresh.txt") == "DearsScene 8\n0\n", "New content invalid");
            Check(!(scene.get()->*Get(History{})).CanUndo() && !(scene.get()->*Get(History{})).CanRedo(), "New history not reset");
        });
        auto reject = [&](const std::string& name, const std::string& contents) {
            test("Open " + name + " preserves complete live state", [&] {
                auto scene = Seed(); Write("../Scenes/rejected.txt", contents);
                Snapshot before(*scene); (scene.get()->*Get(Open{}))("../Scenes/rejected.txt"); before.Unchanged(*scene);
                Check(File("../Scenes/rejected.txt") == contents, "Open modified rejected file");
            });
        };
        test("Open missing file preserves complete live state", [] {
            fs::remove("../Scenes/missing.txt"); auto scene = Seed(); Snapshot before(*scene);
            (scene.get()->*Get(Open{}))("../Scenes/missing.txt"); before.Unchanged(*scene);
        });
        reject("invalid magic", "not a scene\n");
        reject("unsupported version", "DearsScene 9\n0\n");
        reject("zero version", "DearsScene 0\n0\n");
        reject("missing count", "DearsScene 8\n");
        reject("negative count", "DearsScene 8\n-1\n");
        reject("overflow count", "DearsScene 8\n9999999999999999999999999\n");
        reject("non-numeric count", "DearsScene 8\nobjects\n");
        reject("missing record", "DearsScene 8\n1\n");
        reject("partially valid second record", "DearsScene 8\n2\n" + Record(8, "parsed first") + "\"truncated second\"");
        reject("truncated final matrix", Scene(8).substr(0, Scene(8).size() - 8));
        reject("unterminated quote", "DearsScene 8\n1\n\"unfinished name");
        reject("unknown render type", Changed(Scene(8), "DirectionalLight", "UnknownLight"));
        reject("unknown render path", Changed(Scene(8), "Deferred", "UnknownPath"));
        reject("unknown shading model", Changed(Scene(8), "DefaultLit", "UnknownModel"));
        reject("nonfinite material value", Changed(Scene(8), "0.2 0.4 0.1", "nan 0.4 0.1"));
        reject("overflow matrix value", Changed(Scene(8), " 1 0 0 0", " 1e999 0 0 0"));
        reject("unexpected trailing data", Scene(8) + "extra data");
        reject("count smaller than actual records", "DearsScene 8\n1\n" + Record(8, "one") + Record(8, "two"));
        test("Open sharing-denied file preserves complete live state", [] {
            auto scene = Seed(); Write("../Scenes/locked.txt", Scene(8)); Snapshot before(*scene);
            HANDLE lock = CreateFileW(L"../Scenes/locked.txt", GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
            Check(lock != INVALID_HANDLE_VALUE, "fixture lock failed");
            (scene.get()->*Get(Open{}))("../Scenes/locked.txt"); CloseHandle(lock); before.Unchanged(*scene);
        });
        test("Open directory preserves current state", [] {
            auto scene = Seed(); fs::create_directories("../Scenes/not-a-file"); Snapshot before(*scene);
            (scene.get()->*Get(Open{}))("../Scenes/not-a-file"); before.Unchanged(*scene);
        });
        test("ReadScene itself rejects partial input without appending objects", [] {
            auto scene = Seed(); Snapshot before(*scene);
            std::istringstream input("DearsScene 8\n2\n" + Record(8, "first") + "partial");
            (scene.get()->*Get(Read{}))(input); before.Unchanged(*scene);
        });
        test("Restore invalid snapshot keeps scene and selection", [] {
            auto scene = Seed(); Snapshot before(*scene);
            (scene.get()->*Get(Restore{}))("invalid snapshot"); before.Unchanged(*scene);
        });
        for (int version = 1; version <= 8; ++version) test("Open supported scene version " + std::to_string(version), [&] {
            auto scene = Seed(); Write("../Scenes/valid.txt", Scene(version));
            (scene.get()->*Get(Open{}))("../Scenes/valid.txt");
            Check((scene.get()->*Get(Objects{})).size() == 2, "valid scene object count wrong");
            Check((scene.get()->*Get(Names{}))[0] == "loaded 0", "valid scene content wrong");
            Check(scene.get()->*Get(Path{}) == "../Scenes/valid.txt", "valid scene path wrong");
            Check(scene.get()->*Get(Selected{}) == -1, "valid Open kept invalid selection");
            Check(!(scene.get()->*Get(History{})).CanUndo() && !(scene.get()->*Get(History{})).CanRedo(), "Open retained prior history");
            const auto& desc = (scene.get()->*Get(Descs{}))[0];
            Check(desc.renderPath == (version >= 2 ? SceneRenderPath::Deferred : SceneRenderPath::Forward), "legacy render-path default wrong");
            Check(desc.pbrMetallic == (version >= 3 ? .2f : 0.0f), "legacy material default wrong");
            Check(desc.rotationDegrees.x == (version >= 4 ? 10.0f : 0.0f), "legacy rotation default wrong");
            Check(desc.thinFilmThickness == (version >= 6 ? 100 : 1), "legacy thin-film default wrong");
        });
        test("valid scene accepts trailing whitespace and no final newline", [] {
            auto scene = Seed(); auto text = Scene(8); text.pop_back(); Write("../Scenes/no-newline.txt", text);
            (scene.get()->*Get(Open{}))("../Scenes/no-newline.txt");
            Check((scene.get()->*Get(Objects{})).size() == 2, "complete file without newline rejected");
            Write("../Scenes/whitespace.txt", Scene(8) + " \t\r\n\n");
            (scene.get()->*Get(Open{}))("../Scenes/whitespace.txt");
            Check(scene.get()->*Get(Path{}) == "../Scenes/whitespace.txt", "trailing whitespace rejected");
        });
        test("valid empty scene commits", [] {
            auto scene = Seed(); Write("../Scenes/empty.txt", "DearsScene 8\n0\n");
            (scene.get()->*Get(Open{}))("../Scenes/empty.txt");
            Check((scene.get()->*Get(Objects{})).empty() && scene->GetMainRenderItems().empty() && scene->GetShadowRenderItems().empty(), "empty scene failed to commit");
        });
        test("production Serialize Open round trip", [] {
            auto scene = Seed(); const auto text = Text(*scene); Write("../Scenes/roundtrip.txt", text);
            (scene.get()->*Get(Open{}))("../Scenes/roundtrip.txt"); Check(Text(*scene) == text, "round trip changed saved state");
        });
        test("Initialize loads a valid existing default scene through production parser", [] {
            Write("../SavedScene.txt", Scene(8)); DemoScene scene(nullptr); scene.Initialize();
            Check((scene.*Get(Objects{})).size() == 2 && (scene.*Get(Names{}))[0] == "loaded 0", "startup load failed");
            Check(scene.*Get(Path{}) == "../SavedScene.txt", "startup path not committed");
        });
        test("Initialize invalid default file does not commit its path", [] {
            Write("../SavedScene.txt", "invalid default scene"); DemoScene scene(nullptr); scene.Initialize();
            Check((scene.*Get(Objects{})).empty() && (scene.*Get(Path{})).empty(), "startup committed failed input");
            Check(File("../SavedScene.txt") == "invalid default scene" && ErrorPopup(scene), "startup failure lost disk bytes or error");
        });
        test("R1 missing startup default Initialize edit Save preserves edits on disk", [] {
            fs::remove("../SavedScene.txt"); DemoScene scene(nullptr); scene.Initialize();
            AddLight(scene, "saved after first-run startup"); (scene.*Get(Save{}))();
            Check(scene.*Get(Path{}) == "../SavedScene.txt", "missing startup default left no ordinary Save destination");
            Check(fs::is_regular_file("../SavedScene.txt") && FileText("../SavedScene.txt") == Text(scene), "ordinary Save silently dropped startup edits");
            DemoScene reopened(nullptr); (reopened.*Get(Open{}))("../SavedScene.txt");
            Check((reopened.*Get(Objects{})).size() == 1 && (reopened.*Get(Names{}))[0] == "saved after first-run startup", "saved startup edit did not round trip");
        });
        test("R1 existing invalid startup default survives edit and normal Save", [] {
            const std::string original = "invalid startup file must survive";
            Write("../SavedScene.txt", original); DemoScene scene(nullptr); scene.Initialize();
            AddLight(scene, "unsaved protected edit"); (scene.*Get(Save{}))();
            Check((scene.*Get(Path{})).empty() && File("../SavedScene.txt") == original, "invalid startup file became an autosave target");
        });
        test("R1 existing unreadable startup default stays protected after unlock and Save", [] {
            const std::string original = Scene(8); Write("../SavedScene.txt", original);
            HANDLE lock = CreateFileW(L"../SavedScene.txt", GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
            Check(lock != INVALID_HANDLE_VALUE, "startup fixture lock failed");
            DemoScene scene(nullptr); scene.Initialize(); CloseHandle(lock);
            AddLight(scene, "protected unreadable edit"); (scene.*Get(Save{}))();
            Check((scene.*Get(Path{})).empty() && File("../SavedScene.txt") == original, "unreadable startup file became an autosave target");
        });
        test("production Undo Redo restore real scene snapshots", [] {
            auto scene = Seed(); const auto first = Text(*scene); auto& history = scene.get()->*Get(History{});
            history.Reset(first); scene.get()->*Get(Dirty{}) = false;
            (scene.get()->*Get(Objects{}))[0]->ObjectPos = Matrix::CreateTranslation(10, 20, 30);
            const auto second = Text(*scene); history.Commit(second);
            (scene.get()->*Get(Undo{}))(); Check(Text(*scene) == first && history.CanRedo(), "Undo failed");
            (scene.get()->*Get(Redo{}))(); Check(Text(*scene) == second && history.CanUndo(), "Redo failed");
            Check(FileText("../Scenes/current.txt") == second, "Redo save failed in isolated file");
        });
        test("production Play Stop restores edit scene and history", [] {
            auto scene = Seed(); const auto original = Text(*scene); const auto disk = File("../Scenes/current.txt");
            (scene.get()->*Get(Play{}))(true); const auto history = HistoryText(scene.get()->*Get(History{}));
            AddLight(*scene, "runtime-only object");
            (scene.get()->*Get(Objects{}))[0]->ObjectPos = Matrix::CreateTranslation(90, 0, 0);
            (scene.get()->*Get(Play{}))(false);
            Check(!scene->IsPlaying() && Text(*scene) == original, "Stop did not restore edit snapshot");
            Check(HistoryText(scene.get()->*Get(History{})) == history && File("../Scenes/current.txt") == disk, "Play Stop corrupted history or disk");
        });
        test("failed Undo snapshot preserves scene history cursor dirty state and disk", [] {
            auto scene = Seed(); auto& history = scene.get()->*Get(History{});
            history.Reset("invalid undo snapshot"); history.Commit(Text(*scene));
            scene.get()->*Get(Dirty{}) = true; Snapshot before(*scene);
            (scene.get()->*Get(Undo{}))(); before.Unchanged(*scene);
        });
        test("failed Redo snapshot preserves scene history cursor and disk", [] {
            auto scene = Seed(); auto& history = scene.get()->*Get(History{});
            history.Reset(Text(*scene)); history.Commit("invalid redo snapshot"); history.Undo();
            scene.get()->*Get(Dirty{}) = false; Snapshot before(*scene);
            (scene.get()->*Get(Redo{}))(); before.Unchanged(*scene);
        });
        test("failed Stop preserves Play snapshot and complete running state", [] {
            auto scene = Seed(); (scene.get()->*Get(Play{}))(true);
            scene.get()->*Get(PlaySnapshot{}) = "invalid play snapshot"; Snapshot before(*scene);
            (scene.get()->*Get(Play{}))(false); before.Unchanged(*scene);
            Check(scene->IsPlaying() && scene.get()->*Get(PlaySnapshot{}) == "invalid play snapshot", "Stop discarded failed snapshot");
        });
        test("New and Open remain disabled during Play", [] {
            auto scene = Seed(); (scene.get()->*Get(Play{}))(true);
            const auto play = scene.get()->*Get(PlaySnapshot{}); Snapshot before(*scene);
            (scene.get()->*Get(New{}))("ignored-during-play");
            (scene.get()->*Get(Open{}))("../Scenes/missing.txt"); before.Unchanged(*scene);
            Check(scene->IsPlaying() && scene.get()->*Get(PlaySnapshot{}) == play, "file operation corrupted Play");
        });
        test("failed Open reports a visible real ImGui error popup", [] {
            auto scene = Seed(); (scene.get()->*Get(Open{}))("../Scenes/missing.txt");
            Check(ErrorPopup(*scene), "no visible failure message");
        });
        test("New collision reports a visible real ImGui error popup", [] {
            auto scene = Seed(); Write("../Scenes/existing.txt", Scene(8));
            (scene.get()->*Get(New{}))("existing"); Check(ErrorPopup(*scene), "collision has no visible error");
        });
        test("successful Open clears a prior failure message", [] {
            auto scene = Seed(); (scene.get()->*Get(Open{}))("../Scenes/missing.txt");
            Write("../Scenes/valid.txt", Scene(8)); (scene.get()->*Get(Open{}))("../Scenes/valid.txt");
            Check(!ErrorPopup(*scene), "stale error remained after successful Open");
        });
        ImGui::DestroyContext();
    } catch (const std::exception& error) { std::cout << "SETUP_FAILURE " << error.what() << '\n'; return 2; }
    std::cout << "RESULT " << passed << " passed, " << failed << " failed\n";
    return failed ? 1 : 0;
}
