#pragma once
#include <windows.h>
#include "DearsGraphicsEngine.h"
#include "AStar.h"
#include "tempObject.h"
#include "EasingFunc.h"
#include "ObjectPool.h"


class TimeManager;
class InputManager;
class FileManager;
struct ModelBuffer;
struct VSConstantBufferData;
struct VSBoneConstantBufferData;
struct PSConstantBufferData;
class Camera;
class AStar;

class GameEngine
{
public:
	GameEngine(HWND _hWnd, const int _screenWidth, const int _screenHeight);
	~GameEngine();

private:
	HWND m_hWnd;
	int m_screenWidth;
	int m_screenHeight;

	TimeManager* m_pTimeManager;
	InputManager* m_pInputManager;
	FileManager* m_pFileManager;
	DearsGraphicsEngine* m_pDearsGraphicsEngine;
	EasingFunc* tempEasing;

	Camera* tempCamera;
	Camera* lightCamera;

	tempObject* tempObject1;
	tempObject* tempObject2;
	tempObject* tempObject3;
	tempObject* tempObject4;
	tempObject* tempObject5;
	tempObject* tempObject6;
	tempObject* tempObject7;
	tempObject* tempObject8;
	tempObject* tempObject9;

	// AStar
	AStar* tempAStar;
	std::vector<Vector2> tempRawPath;
	Vector3 nextPos;
	size_t pathIndex = 0;

	//UI tweening
	Vector2 uiPoint;
	Vector2 uiPoint2;
public:
	//���� ������Ʈ ������ ���ӿ������� �ڷᱸ��ȭ ���� ����.
	CommonConstantBufferData tempCCConstantBuffer;
	//������ ��ġ�� �����ϴ� commonConstant
	CommonConstantBufferData tempLightCConstantBuffer;

	PsShadowConstantBufferData tempPSShadowConstantData;

	float opacityValue[4] = { 1.0,1.0,1.0,1.0 };
public:
	//�׷��Ƚ� ����, �������� ���� ��ü�� �����Ѵ�.
	void Initialize();

	//������Ʈ�� ����
	void Update();

	//���� �����Լ�
	void Render();

	void GameLoop();

	void Finalize();

	// Ŭ���� �������� ������ ���� ����� �Լ�
private:
	void InitializeManager();
};