#include "DebugRenderer.h"

#include "DearsGraphicsEngine.h"
#include "GraphicsCommon.h"

DebugRenderer::DebugRenderer(DearsGraphicsEngine* graphicsEngine)
	: m_pGraphicsEngine(graphicsEngine)
{
}

void DebugRenderer::Initialize()
{
	m_pDebugModelBuffer = std::make_unique<ModelBuffer>();

	// 디버그 렌더러가 사용하는 FBX 모델만 여기서 등록한다.
	// DearsGraphicsEngine::Initialize()에서 빠져나왔기 때문에,
	// 디버그 도형 리소스가 어디서 준비되는지 한눈에 찾기 쉬워진다.
	m_pGraphicsEngine->AddModel("../TestAsset/", "Debug_Box.fbx");
	m_pGraphicsEngine->AddModel("../TestAsset/", "Debug_Capsule.fbx");
	m_pGraphicsEngine->AddModel("../TestAsset/", "Debug_Sphere.fbx");

	VSConstantBufferData temp;
	m_pDebugModelBuffer->m_pVSConstantBuffer = m_pGraphicsEngine->CreateConstantBuffer(temp);
}

void DebugRenderer::RenderBox(Vector3 size, Vector3 rotation, Vector3 translation)
{
	PrepareDebugModel("Box");

	VSConstantBufferData temp;
	temp.world = (Matrix::CreateScale(size) *
				  Matrix::CreateRotationX(rotation.x) *
				  Matrix::CreateRotationY(rotation.y) *
				  Matrix::CreateRotationZ(rotation.z) *
				  Matrix::CreateTranslation(translation)).Transpose();

	m_pGraphicsEngine->UpdateConstantBuffer(m_pDebugModelBuffer.get(), temp);
	m_pGraphicsEngine->mpRenderer->Render(m_pDebugModelBuffer.get());
}

void DebugRenderer::RenderBox(Matrix size, Matrix rotation, Matrix translation)
{
	PrepareDebugModel("Box");

	VSConstantBufferData temp;
	temp.world = (size * rotation * translation).Transpose();

	m_pGraphicsEngine->UpdateConstantBuffer(m_pDebugModelBuffer.get(), temp);
	m_pGraphicsEngine->mpRenderer->Render(m_pDebugModelBuffer.get());
}

void DebugRenderer::RenderBox(Matrix size, Matrix rotation, Matrix translation, Matrix extraTransform)
{
	PrepareDebugModel("Box");

	VSConstantBufferData temp;
	temp.world = (size * rotation * translation * extraTransform).Transpose();

	m_pGraphicsEngine->UpdateConstantBuffer(m_pDebugModelBuffer.get(), temp);
	m_pGraphicsEngine->mpRenderer->Render(m_pDebugModelBuffer.get());
}

void DebugRenderer::RenderBox(AABB& aabb, Matrix scale, Matrix rotation, Matrix extraTransform)
{
	Vector3 size = {
		aabb.mMax.x - aabb.mMin.x,
		aabb.mMax.y - aabb.mMin.y,
		aabb.mMax.z - aabb.mMin.z,
	};

	Matrix sizeMatrix = Matrix::CreateScale(size);
	sizeMatrix *= scale;

	// AABB의 min/max 중간 지점이 박스의 중심이다.
	// 이 중심으로 이동시켜야 와이어 박스가 원본 모델의 충돌 범위와 같은 위치에 놓인다.
	Vector3 center = {
		(aabb.mMax.x + aabb.mMin.x) / 2.0f,
		(aabb.mMax.y + aabb.mMin.y) / 2.0f,
		(aabb.mMax.z + aabb.mMin.z) / 2.0f
	};

	Matrix translationMatrix = Matrix::CreateTranslation(center);
	RenderBox(sizeMatrix, rotation, translationMatrix, extraTransform);
}

void DebugRenderer::RenderSphere(Vector3 size, Vector3 rotation, Vector3 translation)
{
	PrepareDebugModel("Sphere");

	VSConstantBufferData temp;
	temp.world = (Matrix::CreateScale(size) *
				  Matrix::CreateRotationX(rotation.x) *
				  Matrix::CreateRotationY(rotation.y) *
				  Matrix::CreateRotationZ(rotation.z) *
				  Matrix::CreateTranslation(translation)).Transpose();

	m_pGraphicsEngine->UpdateConstantBuffer(m_pDebugModelBuffer.get(), temp);
	m_pGraphicsEngine->mpRenderer->Render(m_pDebugModelBuffer.get());
}

void DebugRenderer::RenderCapsule(Vector3 size, Vector3 rotation, Vector3 translation)
{
	PrepareDebugModel("Capsule");

	VSConstantBufferData temp;
	temp.world = (Matrix::CreateScale(size) *
				  Matrix::CreateRotationX(rotation.x) *
				  Matrix::CreateRotationY(rotation.y) *
				  Matrix::CreateRotationZ(rotation.z) *
				  Matrix::CreateTranslation(translation)).Transpose();

	m_pGraphicsEngine->UpdateConstantBuffer(m_pDebugModelBuffer.get(), temp);
	m_pGraphicsEngine->mpRenderer->Render(m_pDebugModelBuffer.get());
}

void DebugRenderer::PrepareDebugModel(const std::string& modelName)
{
	// 디버그 도형은 와이어프레임으로 그리기 위해 DebugGeometryPSO를 사용한다.
	// 실제 draw call 직전마다 필요한 모델의 버퍼만 같은 ModelBuffer에 연결한다.
	m_pGraphicsEngine->SetPipelineState(Dears::Graphics::DebugGeometryPSO);
	m_pDebugModelBuffer->m_pVertexBuffer = m_pGraphicsEngine->Get_VertexBuffer(modelName);
	m_pDebugModelBuffer->m_pIndexBuffer = m_pGraphicsEngine->Get_IndexBuffer(modelName);
	m_pDebugModelBuffer->mNumIndices = m_pGraphicsEngine->Get_NumIndex(modelName);
}
