#pragma once
#include "BufferData.h"
#include <directxTK/SimpleMath.h>
#include <string>

using DirectX::SimpleMath::Vector3;

class LightHelper
{
public:
	LightHelper();
	~LightHelper() {};

private:
	int mLightMaxNumber;

public:
	// [PS버퍼구조, 만드려는 빛의 개수] 원하는 Light의 개수를 만들고, CommonConstantBufferData에 전해준다.
	void Initialize(CommonConstantBufferData* _psBufferData, UINT _num);

	// [PS버퍼구조] Light Update, PS 버퍼 업데이트 전에 호출한다.
	void Update(CommonConstantBufferData* _commonBufferData);

	// [PS버퍼구조, 만드려는 빛의 개수] Light의 개수 조정 -> 이거를 추가랑 삭제로 만드는 게 더 낫겠다... -> 라이트의 이름을 추가해줘야 될 거 같다.
	void ChangeLightMaxNum(CommonConstantBufferData* _psBufferData, UINT _num);

	// [PS버퍼구조] 현재 컨스턴트 버퍼의 빛의 개수와 정보 출력
	void PrintInfo(CommonConstantBufferData* _psBufferData);

public:
	// 라이트 내의 성분 전체 설정 [PS버퍼구조, 종류, 세기, 빛 강도 하락의 시작, 빛 강도 하락의 끝, 방향, 위치, spotPower]
	void SetLightSettingAll(CommonConstantBufferData* _psBufferData, UINT _index, LightEnum _lightType, float _strength, float _fallOffStart, 
		float _fallOffEnd, Vector3 _dir, Vector3 _pos, float _spotPower, Vector3 _color);

	void SetDirLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, Vector3 _dir, Vector3 _color);

	void SetPointLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, float _fallOffStart,
		float _fallOffEnd, Vector3 _pos, Vector3 _color);

	void SetSpotLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, float _fallOffStart,
		float _fallOffEnd, Vector3 _dir, Vector3 _pos, float _spotPower, Vector3 _color);

	// 현재 인덱스의 라이트를 NONE으로 바꿉니다.
	void SetLightOff(CommonConstantBufferData* _psBufferData, UINT _index);


	// [PS버퍼구조, index, 빛의 세기(0~1)] 빛의 세기를 설정, 0~1의 값만 넣는다.
	void SetStrength(CommonConstantBufferData* _psBufferData, UINT _index, float _strength);
	// [PS버퍼구조, index, 빛의 종류] 빛의 종류를 설정
	void SetLightType(CommonConstantBufferData* _psBufferData, UINT _index, LightEnum _lightType);
	// [PS버퍼구조, index, 빛의 방향] 빛의 방향 설정, _dir은 단위 벡터로 넣을 것.
	void SetDirection(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _dir);
	// [PS버퍼구조, index, 빛 강도 하락의 시작] 빛의 범위 설정
	void SetFallOffStart(CommonConstantBufferData* _psBufferData, UINT _index, float _distance);
	// [PS버퍼구조, index, 빛 강도 하락의 끝] 빛의 범위 설정 : fall off Start 보다 큰 값
	void SetFallOffEnd(CommonConstantBufferData* _psBufferData, UINT _index, float _length);
	// [PS버퍼구조, index, 빛의 위치] 빛의 위치를 설정
	void SetPosition(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _pos);
	// [PS버퍼구조, index, 빛의 spotPower(0~100)] 빛의 spotPower를 설정 : 클수록 빛이 모아짐
	void SetSpotPower(CommonConstantBufferData* _psBufferData, UINT _index, float _power);
	// [PS버퍼구조, index, 빛의 방향] 빛의 색깔 설정
	void SetLightColor(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _rgb);
	// [PS버퍼구조, 복사할 빛, 복사될 인덱스] 특정 인덱스의 빛 복사하기
	void CopyLight(CommonConstantBufferData* _psBufferData, UINT _copy, UINT _origin);

private:
	// [ PS버퍼구조, index] 함수 내부에서 유효한 인덱스인지 확인하기 위해 만든 함수
	bool CheckIndex(CommonConstantBufferData* _psBufferData, UINT _index);
};