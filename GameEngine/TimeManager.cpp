#include <windows.h>
#include <iostream>
#include "TimeManager.h"

//***************************************************************************************
// GameTimer.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

TimeManager* TimeManager::m_pInstance = nullptr;

TimeManager::TimeManager() : m_secondsPerCount(0.0), m_deltaTime(-1.0), m_baseTime(0), m_pausedTime(0),
							 m_stopTime(0), m_previousTime(0), m_currentTime(0), m_isStopped(false)
{
	__int64 countsPerSeconds;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSeconds); // �����Ϳ� �������� ���̿����� C��Ÿ���� ĳ������ �ϴ� ���� �Ϲ����̴�.
	m_secondsPerCount = 1.0 / static_cast<double>(countsPerSeconds); // �ʴ� Ŭ�� Ƚ�� -> T = 1/f /// f = 1/T
}

// �ν��Ͻ��� ���� ��쿡�� �����ؼ� ��ȯ�Ѵ�.
TimeManager* TimeManager::getInstance()
{
	if (!m_pInstance)
	{
		m_pInstance = new TimeManager();
	}
	return m_pInstance;
}

// �ð谡 ������ �ð��� ī��Ʈ���� �ʰ� Reset()�� ȣ��� �� ����� �� �ð��� ��ȯ.
float TimeManager::TotalTime() const
{
	// �츮�� ���� ���, �츮�� ���� �ķκ��� ���� �ð��� ���� ����.
	// ����, �츮�� ������ �Ͻ� ������ �� ���, (mStopTime - mBaseTime)�� �Ÿ����� �Ͻ� ������ �ð��� ���ԵǸ�, �� �ð��� ������� �ʴ´�.
	// �̸� �����Ϸ���, mStopTime���� �Ͻ� ������ �ð��� �� �� �ִ�:  
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mStopTime    mCurrTime

	if (m_isStopped)
	{
		return static_cast<float>(((m_stopTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
	}

	// (mCurrTime - mBaseTime)�� �Ÿ��� �Ͻ� ������ �ð��� �����ϰ�, �� �ð��� ������� �ʴ´�.
	// �̸� �����Ϸ���, mCurrTime���� �Ͻ� ������ �ð��� �� �� �ִ�.
	//  (mCurrTime - mPausedTime) - mBaseTime 
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mCurrTime

	else
	{
		return static_cast<float>(((m_currentTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
	}
}

float TimeManager::DeltaTime()const
{
	//std::cout << static_cast<float>(m_deltaTime) << std::endl;
	return static_cast<float>(m_deltaTime);
}

float TimeManager::FPS() const
{
	//std::cout << 1.0 / static_cast<float>(m_deltaTime) << std::endl;
	return 1.0f / static_cast<float>(m_deltaTime);
}

void TimeManager::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime); // ���� ���ļ� �˾Ƴ���

	m_baseTime = currTime;
	m_previousTime = currTime;
	m_stopTime = 0;
	m_isStopped = false;
}

// ���⼭ �� �� ���ϰ� ����ϱ� ���� startTime�̶� ���� ��� m_currentTime�� �ᵵ ������, �̷��� TotalTime()�Լ��� ��׷�����.
// �׷��� ���ǻ� �׳� �и��ؼ� ����Ѵ�.
void TimeManager::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	// ������ ���� ���� ��� �� �ð��� �����Ѵ�.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  mBaseTime       mStopTime        startTime     

	if (m_isStopped)
	{
		m_pausedTime += (startTime - m_stopTime); // ����� ���� �� ������ �� �� �ֱ� ������ �Ͻ� ���� �ð��� ������ �����Ѵ�.

		m_previousTime = startTime; // ���� �ð�(��� �� �� ���������� ���� �ð�)�� ���� �ð��� �����ϰ�
		m_stopTime = 0; // stopTime�� ���½�Ų��. -> �Ͻ� ���� �ĺ����� �ð��� �ٽ� �����ϱ� ���� ���°� ����ϰ� �ٲٴ� ���̴�.
		m_isStopped = false;
	}
}

void TimeManager::Stop()
{
	if (!m_isStopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_stopTime = currTime; // ���� ���ļ��� stopTime�� ����
		m_isStopped = true;
	}
}

void TimeManager::Tick()
{
	if (m_isStopped)
	{
		m_deltaTime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_currentTime = currTime;

	// �� �����Ӱ� ���� ������ ������ �ð� ����.
	m_deltaTime = (m_currentTime - m_previousTime) * m_secondsPerCount;

	const float targetDeltaTime = 1.0f / 60.0f; // 60 FPS ����
	while (m_deltaTime < targetDeltaTime)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		m_currentTime = currTime;
		m_deltaTime = (m_currentTime - m_previousTime) * m_secondsPerCount;
	}

	// ���� �������� �غ�.
	m_previousTime = m_currentTime;

	// �������� ����ȭ.
	//  DXSDK�� CDXUTTimer�� ���μ����� ���� ���(power save mode)�� ��ȯ�ǰų�, �ٸ� ���μ����� ���õǰ� �Ǹ�, mDeltaTime�� ������ �� �� �ִٰ� ����Ѵ�.
	if (m_deltaTime < 0.0)
	{
		m_deltaTime = 0.0;
	}
}