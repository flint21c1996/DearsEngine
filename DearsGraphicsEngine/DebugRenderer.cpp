#include "DebugRenderer.h"

#include "DearsGraphicsEngine.h"
#include "GraphicsCommon.h"
#include "Light.h"
#include "RendererHelper.h"

#include <cmath>
#include <cstring>

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

	// 라이트 기즈모는 매 프레임 정점 위치가 달라지므로 CPU_WRITE 가능한 동적 버퍼를 사용한다.
	// MAX_LIGHTS가 작아 4096개 정점이면 여러 원과 원뿔을 한 프레임에 그리기에 충분하다.
	D3D11_BUFFER_DESC lineBufferDesc = {};
	lineBufferDesc.ByteWidth = static_cast<UINT>(sizeof(LineVertex) * 4096);
	lineBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lineBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	lineBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_pGraphicsEngine->m_pDevice->CreateBuffer(&lineBufferDesc, nullptr, m_pLineVertexBuffer.GetAddressOf());

	const std::vector<D3D11_INPUT_ELEMENT_DESC> lineInputElements =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	RendererHelper::CreateVertexShaderAndInputLayout(
		m_pGraphicsEngine->m_pDevice,
		"../DearsGraphicsEngine/Shader/DebugLineVertexShader.hlsl",
		lineInputElements,
		m_pLineVertexShader,
		m_pLineInputLayout);
	RendererHelper::CreatePixelShader(
		m_pGraphicsEngine->m_pDevice,
		"../DearsGraphicsEngine/Shader/DebugLinePixelShader.hlsl",
		m_pLinePixelShader);
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

void DebugRenderer::RenderLightGizmo(
	const Light& light,
	bool drawShadowFrustum,
	float shadowAspect)
{
	m_lineVertices.clear();

	const Vector3 position = light.position;
	Vector3 direction = light.direction;
	if (direction.LengthSquared() < 0.0001f)
	{
		direction = Vector3::UnitZ;
	}
	direction.Normalize();

	// 라이트 오브젝트의 로컬 +Y를 회전한 실제 Up축을 사용한다.
	// 이전처럼 direction의 기울기에 따라 X/Y 기준축을 교체하면 그 경계에서
	// 절두체가 갑자기 90도 회전하므로 Shadow Camera와 동일한 기저를 직접 전달받는다.
	Vector3 lightUp = light.shadowUp;
	if (lightUp.LengthSquared() < 0.0001f || std::abs(lightUp.Dot(direction)) > 0.999f)
	{
		lightUp = std::abs(direction.Dot(Vector3::UnitY)) > 0.95f
			? Vector3::UnitX
			: Vector3::UnitY;
	}
	lightUp.Normalize();
	Vector3 right = lightUp.Cross(direction);
	right.Normalize();
	Vector3 up = direction.Cross(right);
	up.Normalize();

	const float markerSize = 0.35f;
	const Vector4 directionalColor(1.0f, 0.85f, 0.2f, 1.0f);
	const Vector4 pointColor(1.0f, 0.5f, 0.1f, 1.0f);
	const Vector4 spotColor(0.2f, 0.75f, 1.0f, 1.0f);

	// 모든 라이트의 중심에는 3축 십자 표시를 그려 정확한 위치를 알 수 있게 한다.
	const Vector4 centerColor = light.lightType == static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT)
		? directionalColor
		: light.lightType == static_cast<UINT>(LightEnum::POINT_LIGHT) ? pointColor : spotColor;
	if (light.lightType != static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT))
	{
		AddLine(position - Vector3::UnitX * markerSize, position + Vector3::UnitX * markerSize, centerColor);
		AddLine(position - Vector3::UnitY * markerSize, position + Vector3::UnitY * markerSize, centerColor);
		AddLine(position - Vector3::UnitZ * markerSize, position + Vector3::UnitZ * markerSize, centerColor);
	}

	if (light.lightType == static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT))
	{
		// 사용자가 요청한 dirVec는 라이트 위치에서 실제 Far 평면 중심까지 한 줄로 표시한다.
		AddLine(position, position + direction * light.shadowFar, directionalColor);

		if (drawShadowFrustum)
		{
			// 실제 Directional Shadow Camera와 같은 직교 투영 범위를 그린다.
			// Near/Far 사각형의 크기가 같으므로 화면에는 직육면체 형태로 표시된다.
			const float safeAspect = (std::max)(shadowAspect, 0.001f);
			AddOrthographicFrustum(
				position, direction, right, up,
				light.shadowWidth, light.shadowWidth / safeAspect,
				light.shadowNear, light.shadowFar,
				directionalColor);
		}
	}
	else if (light.lightType == static_cast<UINT>(LightEnum::POINT_LIGHT))
	{
		const float radius = (std::max)(light.fallOffEnd, 0.1f);
		AddCircle(position, Vector3::UnitX, Vector3::UnitY, radius, pointColor);
		AddCircle(position, Vector3::UnitX, Vector3::UnitZ, radius, pointColor);
		AddCircle(position, Vector3::UnitY, Vector3::UnitZ, radius, pointColor);
	}
	else if (light.lightType == static_cast<UINT>(LightEnum::SPOT_LIGHT))
	{
		const float length = (std::max)(light.fallOffEnd, 0.1f);
		// 현재 Spot은 각도 대신 pow 지수(Spot Power)를 사용한다.
		// 밝기가 1%가 되는 경계를 시각적 원뿔의 외곽으로 정의한다.
		const float power = (std::max)(light.spotPower, 1.0f);
		const float halfAngle = std::acos(std::pow(0.01f, 1.0f / power));
		const float radius = std::tan(halfAngle) * length;
		const Vector3 endCenter = position + direction * length;
		AddCircle(endCenter, right, up, radius, spotColor);
		AddLine(position, endCenter + right * radius, spotColor);
		AddLine(position, endCenter - right * radius, spotColor);
		AddLine(position, endCenter + up * radius, spotColor);
		AddLine(position, endCenter - up * radius, spotColor);
		AddLine(position, endCenter, spotColor);
	}

	FlushLines();
}

void DebugRenderer::AddLine(Vector3 start, Vector3 end, Vector4 color)
{
	m_lineVertices.push_back({ start, color });
	m_lineVertices.push_back({ end, color });
}

void DebugRenderer::AddCircle(Vector3 center, Vector3 axisX, Vector3 axisY, float radius, Vector4 color, int segments)
{
	for (int segment = 0; segment < segments; ++segment)
	{
		const float angle0 = DirectX::XM_2PI * static_cast<float>(segment) / static_cast<float>(segments);
		const float angle1 = DirectX::XM_2PI * static_cast<float>(segment + 1) / static_cast<float>(segments);
		const Vector3 point0 = center + axisX * (std::cos(angle0) * radius) + axisY * (std::sin(angle0) * radius);
		const Vector3 point1 = center + axisX * (std::cos(angle1) * radius) + axisY * (std::sin(angle1) * radius);
		AddLine(point0, point1, color);
	}
}

void DebugRenderer::AddPerspectiveFrustum(
	Vector3 position,
	Vector3 direction,
	Vector3 right,
	Vector3 up,
	float fovYDegrees,
	float aspect,
	float nearDistance,
	float farDistance,
	Vector4 color)
{
	// Perspective Shadow Camera의 Near/Far 평면 크기를 FOV와 거리로 계산한다.
	// Near/Far 사각형 8개 변과 두 평면을 연결하는 4개 변, 총 12개 선이 절두체를 만든다.
	const float halfFovRadians = DirectX::XMConvertToRadians(fovYDegrees) * 0.5f;
	const float nearHalfHeight = std::tan(halfFovRadians) * nearDistance;
	const float nearHalfWidth = nearHalfHeight * aspect;
	const float farHalfHeight = std::tan(halfFovRadians) * farDistance;
	const float farHalfWidth = farHalfHeight * aspect;

	const Vector3 nearCenter = position + direction * nearDistance;
	const Vector3 farCenter = position + direction * farDistance;
	const Vector3 nearCorners[4] =
	{
		nearCenter - right * nearHalfWidth + up * nearHalfHeight,
		nearCenter + right * nearHalfWidth + up * nearHalfHeight,
		nearCenter + right * nearHalfWidth - up * nearHalfHeight,
		nearCenter - right * nearHalfWidth - up * nearHalfHeight,
	};
	const Vector3 farCorners[4] =
	{
		farCenter - right * farHalfWidth + up * farHalfHeight,
		farCenter + right * farHalfWidth + up * farHalfHeight,
		farCenter + right * farHalfWidth - up * farHalfHeight,
		farCenter - right * farHalfWidth - up * farHalfHeight,
	};

	for (int corner = 0; corner < 4; ++corner)
	{
		const int nextCorner = (corner + 1) % 4;
		AddLine(nearCorners[corner], nearCorners[nextCorner], color);
		AddLine(farCorners[corner], farCorners[nextCorner], color);
		AddLine(nearCorners[corner], farCorners[corner], color);
	}
}

void DebugRenderer::AddOrthographicFrustum(
	Vector3 position,
	Vector3 direction,
	Vector3 right,
	Vector3 up,
	float width,
	float height,
	float nearDistance,
	float farDistance,
	Vector4 color)
{
	// 직교 투영은 거리에 관계없이 Near/Far 평면의 가로/세로 크기가 같다.
	// 따라서 두 사각형과 대응하는 네 모서리를 연결하면 Directional Shadow가
	// 실제로 depth를 기록하는 직육면체 범위를 월드에서 그대로 볼 수 있다.
	const float halfWidth = width * 0.5f;
	const float halfHeight = height * 0.5f;
	const Vector3 nearCenter = position + direction * nearDistance;
	const Vector3 farCenter = position + direction * farDistance;
	const Vector3 nearCorners[4] =
	{
		nearCenter - right * halfWidth + up * halfHeight,
		nearCenter + right * halfWidth + up * halfHeight,
		nearCenter + right * halfWidth - up * halfHeight,
		nearCenter - right * halfWidth - up * halfHeight,
	};
	const Vector3 farCorners[4] =
	{
		farCenter - right * halfWidth + up * halfHeight,
		farCenter + right * halfWidth + up * halfHeight,
		farCenter + right * halfWidth - up * halfHeight,
		farCenter - right * halfWidth - up * halfHeight,
	};

	for (int corner = 0; corner < 4; ++corner)
	{
		const int nextCorner = (corner + 1) % 4;
		AddLine(nearCorners[corner], nearCorners[nextCorner], color);
		AddLine(farCorners[corner], farCorners[nextCorner], color);
		AddLine(nearCorners[corner], farCorners[corner], color);
	}
}

void DebugRenderer::FlushLines()
{
	if (m_lineVertices.empty() || !m_pLineVertexBuffer || !m_pLineVertexShader || !m_pLinePixelShader)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	if (FAILED(m_pGraphicsEngine->m_pDeviceContext->Map(
		m_pLineVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	{
		return;
	}
	memcpy(mapped.pData, m_lineVertices.data(), sizeof(LineVertex) * m_lineVertices.size());
	m_pGraphicsEngine->m_pDeviceContext->Unmap(m_pLineVertexBuffer.Get(), 0);

	// 기존 Debug PSO의 depth/raster/blend 상태를 재사용하고 선 전용 셰이더만 교체한다.
	m_pGraphicsEngine->SetPipelineState(Dears::Graphics::DebugGeometryPSO);
	UINT stride = sizeof(LineVertex);
	UINT offset = 0;
	m_pGraphicsEngine->m_pDeviceContext->IASetInputLayout(m_pLineInputLayout.Get());
	m_pGraphicsEngine->m_pDeviceContext->IASetVertexBuffers(0, 1, m_pLineVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pGraphicsEngine->m_pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	m_pGraphicsEngine->m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_pGraphicsEngine->m_pDeviceContext->VSSetShader(m_pLineVertexShader.Get(), nullptr, 0);
	m_pGraphicsEngine->m_pDeviceContext->PSSetShader(m_pLinePixelShader.Get(), nullptr, 0);
	m_pGraphicsEngine->m_pDeviceContext->Draw(static_cast<UINT>(m_lineVertices.size()), 0);
}
