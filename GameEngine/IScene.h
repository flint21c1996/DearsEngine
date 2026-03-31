#pragma once

#include <directxtk/SimpleMath.h>

class InputManager;
class EasingFunc;
class RenderObject;

using DirectX::SimpleMath::Vector2;

// IScene defines the minimum scene contract GameEngine depends on.
// Concrete scenes can own their own objects, editor panels, and test inputs.
class IScene
{
public:
	virtual ~IScene() = default;

	virtual void Initialize() = 0;
	virtual void RegisterEditorPanels(int screenWidth) = 0;
	virtual void Update(float deltaTime) = 0;

	virtual void HandleDemoInput(InputManager& inputManager) = 0;
	virtual void HandlePresentationInput(InputManager& inputManager, EasingFunc& easingFunc, float deltaTime) = 0;
	virtual void HandleRenderInput(InputManager& inputManager) = 0;

	virtual RenderObject* GetCharacter() const = 0;
	virtual RenderObject* GetWeapon() const = 0;
	virtual RenderObject* GetBillboard() const = 0;
	virtual RenderObject* GetPbrSphere() const = 0;
	virtual RenderObject* GetFloor() const = 0;
	virtual RenderObject* GetWater() const = 0;
	virtual RenderObject* GetTerrain() const = 0;
	virtual RenderObject* GetCubeMap() const = 0;

	virtual const Vector2& GetPrimaryUiPoint() const = 0;
	virtual const Vector2& GetSecondaryUiPoint() const = 0;
};
