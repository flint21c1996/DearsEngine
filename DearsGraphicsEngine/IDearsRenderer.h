#pragma once

#include <string>

#include "RenderContext.h"
#include "RenderPassIO.h"

class RenderPipeline;
struct ModelBuffer;
struct Light;

// 메시 한 개를 "어떤 셰이더/파이프라인 조합으로" 그릴지 고른다.
//
// 예전에는 Rend_Model, Rend_PBR, Rend_ThinFilm처럼 종류마다 함수가 하나씩 있어서
// 새 재질이 생길 때마다 그래픽스 엔진의 public 함수가 늘어났다.
// 이제는 DrawMesh() 하나에 이 값을 넘기므로, 새 종류는 enum 값과 내부 분기만 추가하면 된다.
enum class MeshDrawMode
{
	Static,
	Skinned,
	Equipment,
	Pbr,
	ThinFilm,
	// 조명 계산 없이 Albedo/Normal/Material을 G-Buffer에 기록한다. Geometry 패스 전용이다.
	DeferredGeometry,
	CubeMap,
	Billboard,
	Opacity,
	Water,
	Instanced,
	// 스텐실 펀칭 + 확대 메시로 선택 외곽선을 그린다.
	Outline,
};

// Shadow Map에 깊이만 기록할 때 사용할 정점 처리 방식이다.
enum class ShadowCasterMode
{
	Static,
	Skinned,
	Equipment,
	// Height Map으로 정점이 변형되는 PBR 메시용. 화면과 그림자의 실루엣을 일치시킨다.
	Pbr,
};

// 게임 엔진과 렌더 패스가 그래픽스 엔진을 사용할 때 의존하는 인터페이스이다.
//
// 이 헤더에는 DX11 타입이 하나도 없다. 프레임을 돌리고, 패스를 구성하고,
// 패스 안에서 그리는 데 필요한 함수만 모아 두었기 때문에:
// - 패스 코드와 RenderDispatcher는 DearsGraphicsEngine(DX11 구현)을 include하지 않아도 되고,
// - 가짜 구현을 넣어 DirectX 디바이스 없이 패스 구성/순서를 테스트할 수 있으며,
// - 나중에 DX12/Vulkan 백엔드를 만들면 이 인터페이스의 구현만 추가하면 된다.
//
// 아직 인터페이스 밖에 남아 있는 것:
// 리소스 로딩(AddModel 등), 상수 버퍼 생성/갱신, UI/ImGui 함수는 DX11 타입을 직접 노출하고 있어서
// DearsGraphicsEngine 구체 클래스에 그대로 있다. 리소스 핸들을 도입한 뒤에 옮길 대상이다.
class IDearsRenderer
{
public:
	virtual ~IDearsRenderer() = default;

	/// 프레임 -------------------------------------------------------------------------

	// 게임 엔진이 패스를 등록/삭제/재배치하는 목록이다.
	virtual RenderPipeline& GetRenderPipeline() = 0;

	// BeginRender -> 파이프라인에 등록된 패스 실행 -> EndRender(Present)까지 한 프레임을 처리한다.
	virtual void RenderFrame() = 0;

	/// 렌더 타깃 ----------------------------------------------------------------------

	// 패스가 출력으로 쓰거나 입력으로 읽을 오프스크린 타깃을 만든다.
	// 같은 이름이 이미 있으면 새로 만들지 않고 기존 핸들을 돌려준다. 실패하면 유효하지 않은 핸들을 돌려준다.
	// 타깃은 렌더러가 소유하며 렌더러가 파괴될 때 함께 해제된다.
	virtual RenderTargetHandle CreateRenderTarget(const RenderTargetDesc& desc) = 0;
	virtual RenderTargetHandle FindRenderTarget(const std::string& name) const = 0;

	/// 패스 상태 ----------------------------------------------------------------------

	// renderContext.io(없으면 패스 종류의 기본 구성)에 맞게 렌더 타깃/입력 리소스를 바인딩하고
	// 카메라와 공통 상수 버퍼를 갱신한다.
	// RenderPassDesc::applyContext가 true인 패스는 파이프라인이 대신 호출해 준다.
	virtual void ApplyRenderContext(const RenderContext& renderContext) = 0;

	// Shadow writes use the light index in the common buffer. Begin* selects
	// one 2D slice or cube face (+X, -X, +Y, -Y, +Z, -Z). EndShadowPass
	// releases the DSV and seals writes. They become readable only when the
	// enclosing typed Shadow producer returns successfully.
	// Typed pipeline lifecycle; skipped/disabled producers leave no current maps.
	virtual void BeginShadowFrame() {}
	virtual void BeginShadowProduction() {}
	virtual void EndShadowProduction(bool success) {}

	virtual void BeginTwoDimensionalShadowPass(unsigned int lightIndex) = 0;
	virtual void BeginPointShadowFace(unsigned int lightIndex, unsigned int faceIndex) = 0;
	virtual void EndShadowPass() = 0;

	/// 그리기 -------------------------------------------------------------------------

	virtual void DrawMesh(ModelBuffer* modelBuffer, MeshDrawMode mode) = 0;
	// ShaderCatalog에 등록된 이름으로 Vertex/Pixel Shader를 직접 골라서 그린다.
	// 에디터에서 오브젝트마다 셰이더를 바꿀 때 쓰는 경로이다.
	// 이름이 목록에 없거나 두 셰이더의 계열이 달라 조합할 수 없으면 그리지 않고 false를 돌려준다.
	virtual bool DrawMeshWithShaders(
		ModelBuffer* modelBuffer,
		const std::string& vertexShader,
		const std::string& pixelShader) = 0;
	virtual void DrawShadowCaster(ModelBuffer* modelBuffer, ShadowCasterMode mode) = 0;

	// G-Buffer를 읽어 화면 전체 삼각형으로 최종 조명을 계산한다. Lighting 패스에서 호출한다.
	virtual void DrawDeferredLighting() = 0;
	virtual void DrawParticles() = 0;
	virtual void DrawPostProcess() = 0;
	virtual void DrawLightGizmo(const Light& light, bool drawShadowFrustum, float shadowAspect) = 0;
};
