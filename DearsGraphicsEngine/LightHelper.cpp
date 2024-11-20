#pragma once
#include "LightHelper.h"
#include <iostream>

LightHelper::LightHelper()
{
	mLightMaxNumber = 0;
}

void LightHelper::Initialize(CommonConstantBufferData* _psBufferData, UINT _num)
{
	if (_num < 0)
	{
		std::cout << "Error - ChangeLightMaxNum() - 0 이하 Light 개수 입력 불가" << std::endl;
		return;
	}

	mLightMaxNumber = _num;
	_psBufferData->lightNum = mLightMaxNumber;
}

void LightHelper::Update(CommonConstantBufferData* _commonBufferData)
{
	for (UINT i = 0; i < MAX_LIGHTS; i++)
	{

	}
}

void LightHelper::ChangeLightMaxNum(CommonConstantBufferData* _psBufferData, UINT _num)
{
	if (_num < 0)
	{
		std::cout << "Error - ChangeLightMaxNum() - 0 이하 Light 개수 입력 불가" << std::endl;
		return;
	}
	_psBufferData->lightNum = _num;
}


void LightHelper::PrintInfo(CommonConstantBufferData* _psBufferData)
{
	std::cout << "Max Light : " << _psBufferData->lightNum << std::endl;

	std::string tempType = "NoData";

	for (UINT i = 0; i < _psBufferData->lightNum; i++)
	{
		switch (_psBufferData->light[i].lightType)
		{
		case 0:
			tempType = "NONE";
			break;
		case 1:
			tempType = "Directional";
			break;
		case 2:
			tempType = "Point";
			break;
		case 3:
			tempType = "Spot";
			break;
		default:
			tempType = "Error";
			break;
		}
		std::cout << "[" << i << "] " << "Type: " << tempType << ", Strength : " << _psBufferData->light[i].strength\
			<< ", Direction: (" << _psBufferData->light[i].direction.x << ", " << _psBufferData->light[i].direction.y << ", " << _psBufferData->light[i].direction.z\
			<< "), FallOffEnd: " << _psBufferData->light[i].fallOffEnd\
			<< ", Position: (" << _psBufferData->light[i].position.x << ", " << _psBufferData->light[i].position.y << ", " << _psBufferData->light[i].position.z\
			<< "), SpotPower: " << _psBufferData->light[i].spotPower << std::endl;
	}
}

void LightHelper::SetLightSettingAll(CommonConstantBufferData* _psBufferData, UINT _index, LightEnum _lightType, float _strength, float _fallOffStart, 
									 float _fallOffEnd, Vector3 _dir, Vector3 _pos, float _spotPower, Vector3 _color)
{
	_psBufferData->light[_index].lightType = static_cast<UINT>(_lightType);
	_psBufferData->light[_index].strength = _strength;
	_psBufferData->light[_index].fallOffStart = _fallOffStart;
	_psBufferData->light[_index].fallOffEnd = _fallOffEnd;
	_psBufferData->light[_index].direction = _dir;
	_psBufferData->light[_index].position =  _pos;
	_psBufferData->light[_index].spotPower = _spotPower;
	_psBufferData->light[_index].lightColor = _color;
}

void LightHelper::SetDirLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, Vector3 _dir, Vector3 _color)
{
	_psBufferData->light[_index].lightType = static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT);
	_psBufferData->light[_index].strength = _strength;
	_psBufferData->light[_index].direction = _dir;
	_psBufferData->light[_index].lightColor = _color;
	_psBufferData->light[_index].type = 1;
}

void LightHelper::SetPointLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, float _fallOffStart, float _fallOffEnd, 
								Vector3 _pos, Vector3 _color)
{
	_psBufferData->light[_index].lightType = static_cast<UINT>(LightEnum::POINT_LIGHT);
	_psBufferData->light[_index].strength = _strength;
	_psBufferData->light[_index].fallOffStart = _fallOffStart;
	_psBufferData->light[_index].fallOffEnd = _fallOffEnd;
	_psBufferData->light[_index].position = _pos;
	_psBufferData->light[_index].lightColor = _color;
	_psBufferData->light[_index].type = 3;

}

void LightHelper::SetSpotLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, float _fallOffStart, float _fallOffEnd, 
							   Vector3 _dir, Vector3 _pos, float _spotPower, Vector3 _color)
{
	_psBufferData->light[_index].lightType = static_cast<UINT>(LightEnum::SPOT_LIGHT);
	_psBufferData->light[_index].strength = _strength;
	_psBufferData->light[_index].fallOffStart = _fallOffStart;
	_psBufferData->light[_index].fallOffEnd = _fallOffEnd;
	_psBufferData->light[_index].direction = _dir;
	_psBufferData->light[_index].position = _pos;
	_psBufferData->light[_index].spotPower = _spotPower;
	_psBufferData->light[_index].lightColor = _color;
	_psBufferData->light[_index].type = 2;

}

void LightHelper::SetLightOff(CommonConstantBufferData* _psBufferData, UINT _index)
{
	if (!CheckIndex(_psBufferData, _index))
	{
		return;
	}

	_psBufferData->light[_index].lightType = static_cast<UINT>(LightEnum::NONE);
}

void LightHelper::SetStrength(CommonConstantBufferData* _psBufferData, UINT _index, float _strength)
{
	if (!CheckIndex(_psBufferData, _index))
	{
		return;
	}

	if ((_strength >= 0.0f) && (_strength <= 1.0f))
	{
		_psBufferData->light[_index].strength = _strength;
	}
	else
	{
		std::cout << "Error - SetStrength() : Light의 세기는 0.0f ~ 1.0f 값이어야 합니다." << std::endl;
	}
}

void LightHelper::SetLightType(CommonConstantBufferData* _psBufferData, UINT _index, LightEnum _lightType)
{
	if (!CheckIndex(_psBufferData, _index))
	{
		return;
	}

	_psBufferData->light[_index].lightType = static_cast<UINT>(_lightType);
}

void LightHelper::SetDirection(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _dir)
{
	if (!CheckIndex(_psBufferData, _index))
	{
		return;
	}

	_psBufferData->light[_index].direction = _dir;
}

void LightHelper::SetFallOffStart(CommonConstantBufferData* _psBufferData, UINT _index, float _distance)
{
	if (!CheckIndex(_psBufferData, _index))
	{
		return;
	}

	if (_distance > 0.0f)
	{
		_psBufferData->light[_index].fallOffStart = _distance;
	}
	else
	{
		std::cout << "Error - SetFallOffStart() : Light의 범위는 0.0f 이상이어야 합니다." << std::endl;
	}
}

void LightHelper::SetFallOffEnd(CommonConstantBufferData* _psBufferData, UINT _index, float _length)
{
	if (!CheckIndex(_psBufferData, _index) && (_psBufferData->light[_index].fallOffStart > _psBufferData->light[_index].fallOffEnd))
	{
		return;
	}

	if (_length > _psBufferData->light[_index].fallOffStart)
	{
		_psBufferData->light[_index].fallOffEnd = _length;
	}
	else
	{
		std::cout << "Error - SetFallOffEnd() : Light의 범위는 0.0f 이상 / FallOffStart < FallOffEnd" << std::endl;
	}
}

void LightHelper::SetPosition(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _pos)
{
	_psBufferData->light[_index].position = _pos;
}

void LightHelper::SetSpotPower(CommonConstantBufferData* _psBufferData, UINT _index, float _power)
{
	if (!CheckIndex(_psBufferData, _index))
	{
		return;
	}

	if ((_power >= 0.0f) && (_power <= 100.0f))
	{
		_psBufferData->light[_index].spotPower = _power;
	}
	else
	{
		std::cout << "Error - SetSpotPower() : Light의 spotPower는 0.0f ~ 100.0f 값이어야 합니다." << std::endl;
	}
}

void LightHelper::SetLightColor(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _rgb)
{
	if (!CheckIndex(_psBufferData, _index))
	{
		return;
	}
	_psBufferData->light[_index].lightColor = _rgb;
}

void LightHelper::CopyLight(CommonConstantBufferData* _psBufferData, UINT _origin, UINT _copy)
{
	if (!CheckIndex(_psBufferData, _origin) || !CheckIndex(_psBufferData, _copy))
	{
		return;
	}
	_psBufferData->light[_copy] = _psBufferData->light[_origin];
}

bool LightHelper::CheckIndex(CommonConstantBufferData* _psBufferData, UINT _index)
{
	if (_index >= _psBufferData->lightNum)
	{
		std::cout << "Error - Light Max Number Error!" << std::endl;
		return false;
	}

	return true;
}
