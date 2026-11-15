#pragma once

#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

#include "IScene.h"
#include "ShaderCatalog.h"

// "이 오브젝트를 어떻게 그릴지"를 한 덩어리로 묶은 에셋이다.
//
// 오브젝트마다 Vertex Shader, Pixel Shader, 렌더 경로를 하나씩 고르는 대신,
// 조합을 미리 프리셋으로 만들어 두고 오브젝트에는 프리셋 하나만 끌어다 놓는다.
// 언리얼의 머티리얼 에셋처럼 프리셋을 고치면 그 프리셋을 쓰는 모든 오브젝트가 같이 바뀐다.
struct RenderPreset
{
	// 라이브러리 안에서 고유한 이름이다. 오브젝트와 씬 파일은 이 이름으로 프리셋을 참조한다.
	std::string name;

	// ShaderCatalog의 셰이더 이름이다. 비어 있으면 "Auto":
	// 프리셋을 받은 오브젝트의 종류(Static/Skinned/PBR 등)에 맞는 기본 셰이더를 쓴다.
	// Vertex Shader를 Auto로 두면 같은 프리셋을 정적 메시와 스키닝 메시에 함께 쓸 수 있다.
	std::string vertexShader;
	std::string pixelShader;

	// Deferred는 PBR 계열 셰이더에서만 실제로 적용된다. (RenderRouting.h의 ResolveMeshRoute 참고)
	SceneRenderPath renderPath = SceneRenderPath::Forward;
};

// 프리셋 목록을 소유하고 파일로 저장/복원한다.
// ImGui나 그래픽스 엔진을 모르는 순수 데이터 클래스라서 창 없이 테스트할 수 있다.
class RenderPresetLibrary
{
public:
	const std::vector<RenderPreset>& GetPresets() const { return m_presets; }
	bool IsEmpty() const { return m_presets.empty(); }

	const RenderPreset* Find(const std::string& name) const
	{
		for (const RenderPreset& preset : m_presets)
		{
			if (preset.name == name)
			{
				return &preset;
			}
		}
		return nullptr;
	}

	// 프리셋을 추가하고 실제로 등록된 이름을 돌려준다.
	// 같은 이름이 이미 있으면 "Name 2", "Name 3"처럼 번호를 붙여 고유하게 만든다.
	std::string Add(RenderPreset preset)
	{
		Normalize(preset);
		preset.name = MakeUniqueName(preset.name.empty() ? "New Preset" : preset.name);
		m_presets.push_back(preset);
		return m_presets.back().name;
	}

	// 이름은 그대로 두고 내용(셰이더, 렌더 경로)만 바꾼다.
	bool Update(const std::string& name, RenderPreset values)
	{
		RenderPreset* preset = FindMutable(name);
		if (!preset)
		{
			return false;
		}

		values.name = name;
		Normalize(values);
		*preset = values;
		return true;
	}

	// 빈 이름이나 이미 있는 이름으로는 바꿀 수 없다.
	bool Rename(const std::string& oldName, const std::string& newName)
	{
		if (newName.empty() || oldName == newName || Find(newName))
		{
			return false;
		}

		RenderPreset* preset = FindMutable(oldName);
		if (!preset)
		{
			return false;
		}

		preset->name = newName;
		return true;
	}

	bool Remove(const std::string& name)
	{
		for (size_t index = 0; index < m_presets.size(); ++index)
		{
			if (m_presets[index].name == name)
			{
				m_presets.erase(m_presets.begin() + static_cast<std::ptrdiff_t>(index));
				return true;
			}
		}
		return false;
	}

	void Clear() { m_presets.clear(); }

	// 처음 실행했을 때 바로 끌어다 쓸 수 있는 기본 프리셋들이다.
	void AddDefaults()
	{
		Add({ "Lit", "", ShaderCatalog::PixelShader::Lit, SceneRenderPath::Forward });
		Add({ "Unlit", "", ShaderCatalog::PixelShader::Unlit, SceneRenderPath::Forward });
		Add({ "PBR Forward", ShaderCatalog::VertexShader::Pbr, ShaderCatalog::PixelShader::PbrLit, SceneRenderPath::Forward });
		Add({ "PBR Deferred", ShaderCatalog::VertexShader::Pbr, "", SceneRenderPath::Deferred });
		Add({ "Thin Film", ShaderCatalog::VertexShader::Pbr, ShaderCatalog::PixelShader::ThinFilm, SceneRenderPath::Forward });
	}

	bool Save(const std::string& path) const
	{
		std::ofstream file(path, std::ios::trunc);
		if (!file.is_open())
		{
			return false;
		}

		file << "DearsRenderPresets 1\n";
		file << m_presets.size() << '\n';
		for (const RenderPreset& preset : m_presets)
		{
			file << std::quoted(preset.name) << ' '
				<< std::quoted(preset.vertexShader) << ' '
				<< std::quoted(preset.pixelShader) << ' '
				<< (preset.renderPath == SceneRenderPath::Deferred ? "Deferred" : "Forward") << '\n';
		}
		return file.good();
	}

	// 파일이 없거나 형식이 다르면 false를 돌려주고 현재 목록을 건드리지 않는다.
	bool Load(const std::string& path)
	{
		std::ifstream file(path);
		if (!file.is_open())
		{
			return false;
		}

		std::string magic;
		int version = 0;
		size_t count = 0;
		if (!(file >> magic >> version >> count) || magic != "DearsRenderPresets" || version != 1)
		{
			return false;
		}

		std::vector<RenderPreset> loaded;
		for (size_t index = 0; index < count; ++index)
		{
			RenderPreset preset;
			std::string renderPathName;
			if (!(file >> std::quoted(preset.name)
				>> std::quoted(preset.vertexShader)
				>> std::quoted(preset.pixelShader)
				>> renderPathName))
			{
				return false;
			}
			preset.renderPath = renderPathName == "Deferred"
				? SceneRenderPath::Deferred
				: SceneRenderPath::Forward;
			loaded.push_back(preset);
		}

		// 손으로 고친 파일에 중복 이름이나 없는 셰이더가 있어도 Add()가 바로잡는다.
		m_presets.clear();
		for (RenderPreset& preset : loaded)
		{
			Add(preset);
		}
		return true;
	}

	// 프리셋 내용을 "실제로 만들 수 있는 조합"으로 고친다.
	// - 목록에 없는 셰이더 이름은 Auto(빈 문자열)로 바꾼다.
	// - G-Buffer 기록용 셰이더는 Deferred 경로가 자동으로 고르므로 직접 지정을 받지 않는다.
	// - Vertex Shader를 직접 정했는데 Pixel Shader의 계열이 다르면 Pixel Shader를 Auto로 바꾼다.
	static void Normalize(RenderPreset& preset)
	{
		const ShaderInfo* vertexShader = ShaderCatalog::FindVertexShader(preset.vertexShader);
		if (!vertexShader)
		{
			preset.vertexShader.clear();
		}

		const ShaderInfo* pixelShader = ShaderCatalog::FindPixelShader(preset.pixelShader);
		if (!pixelShader || pixelShader->writesGBuffer ||
			(vertexShader && pixelShader->linkage != vertexShader->linkage))
		{
			preset.pixelShader.clear();
		}
	}

private:
	RenderPreset* FindMutable(const std::string& name)
	{
		for (RenderPreset& preset : m_presets)
		{
			if (preset.name == name)
			{
				return &preset;
			}
		}
		return nullptr;
	}

	std::string MakeUniqueName(const std::string& baseName) const
	{
		if (!Find(baseName))
		{
			return baseName;
		}

		for (int suffix = 2;; ++suffix)
		{
			const std::string candidate = baseName + " " + std::to_string(suffix);
			if (!Find(candidate))
			{
				return candidate;
			}
		}
	}

	std::vector<RenderPreset> m_presets;
};

// 프리셋 브라우저에서 끌어 온 항목을 드롭 대상이 알아보는 ImGui 드래그 페이로드 이름이다.
// 페이로드 내용은 널 문자로 끝나는 프리셋 이름이다.
constexpr const char* RenderPresetDragPayloadType = "DEARS_RENDER_PRESET";
