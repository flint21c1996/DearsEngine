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
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSeconds); // 포인터와 비포인터 사이에서는 C스타일의 캐스팅을 하는 것이 일반적이다.
	m_secondsPerCount = 1.0 / static_cast<double>(countsPerSeconds); // 초당 클록 횟수 -> T = 1/f /// f = 1/T
}

// 인스턴스가 없는 경우에만 생성해서 반환한다.
TimeManager* TimeManager::getInstance()
{
	if (!m_pInstance)
	{
		m_pInstance = new TimeManager();
	}
	return m_pInstance;
}

// 시계가 정지된 시간을 카운트하지 않고 Reset()이 호출된 후 경과한 총 시간을 반환.
float TimeManager::TotalTime() const
{
	// 우리가 멈춘 경우, 우리가 멈춘 후로부터 지난 시간을 세지 말자.
	// 또한, 우리가 이전에 일시 중지를 한 경우, (mStopTime - mBaseTime)의 거리에는 일시 중지된 시간이 포함되며, 이 시간은 계산하지 않는다.
	// 이를 수정하려면, mStopTime에서 일시 중지된 시간을 뺄 수 있다:  
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mStopTime    mCurrTime

	if (m_isStopped)
	{
		return static_cast<float>(((m_stopTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
	}

	// (mCurrTime - mBaseTime)의 거리는 일시 중지된 시간을 포함하고, 이 시간은 계산하지 않는다.
	// 이를 수정하려면, mCurrTime에서 일시 중지된 시간을 뺄 수 있다.
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
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime); // 현재 주파수 알아내기

	m_baseTime = currTime;
	m_previousTime = currTime;
	m_stopTime = 0;
	m_isStopped = false;
}

// 여기서 좀 더 편하게 사용하기 위해 startTime이란 변수 대신 m_currentTime을 써도 되지만, 이러면 TotalTime()함수가 어그러진다.
// 그래서 편의상 그냥 분리해서 사용한다.
void TimeManager::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	// 정지와 시작 간에 경과 된 시간을 누적한다.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  mBaseTime       mStopTime        startTime     

	if (m_isStopped)
	{
		m_pausedTime += (startTime - m_stopTime); // 사람이 여러 번 정지를 할 수 있기 때문에 일시 정지 시간은 무조건 누적한다.

		m_previousTime = startTime; // 시작 시간(사실 상 이 시점에서는 현재 시간)을 이전 시간에 저장하고
		m_stopTime = 0; // stopTime을 리셋시킨다. -> 일시 정지 후부터의 시간을 다시 측정하기 위해 리셋과 비슷하게 바꾸는 것이다.
		m_isStopped = false;
	}
}

void TimeManager::Stop()
{
	if (!m_isStopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_stopTime = currTime; // 현재 주파수를 stopTime에 저장
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

	// 이 프레임과 이전 프레임 사이의 시간 차이.
	m_deltaTime = (m_currentTime - m_previousTime) * m_secondsPerCount;

	const float targetDeltaTime = 1.0f / 60.0f; // 60 FPS 기준
	while (m_deltaTime < targetDeltaTime)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		m_currentTime = currTime;
		m_deltaTime = (m_currentTime - m_previousTime) * m_secondsPerCount;
	}

	// 다음 프레임을 준비.
	m_previousTime = m_currentTime;

	// 강제적인 음수화.
	//  DXSDK의 CDXUTTimer는 프로세스가 절전 모드(power save mode)로 전환되거나, 다른 프로세서로 셔플되게 되면, mDeltaTime이 음수가 될 수 있다고 언급한다.
	if (m_deltaTime < 0.0)
	{
		m_deltaTime = 0.0;
	}
}