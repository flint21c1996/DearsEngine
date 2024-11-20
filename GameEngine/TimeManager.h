#pragma once

/// <summary>
/// �̱������� �����ߴ�.
/// 2024.4.23.
/// </summary>
class TimeManager
{
public:
	TimeManager();

private:
	double m_secondsPerCount;
	double m_deltaTime;

	__int64 m_baseTime;
	__int64 m_pausedTime;
	__int64 m_stopTime;
	__int64 m_previousTime;
	__int64 m_currentTime;

	bool m_isStopped;

public:
	// �̱��� ��ü�� ������ �ν��Ͻ��� �����ϱ� ���� ���� ��� ����
	static TimeManager* m_pInstance;

public:
	// ���� ��� �Լ��� ���� �̱��� ��ü�� ���� �� �ֵ��� ��
	static TimeManager* getInstance();

	float TotalTime() const;  // in seconds
	float DeltaTime() const; // in seconds, ������ �� �ð�
	float FPS() const; // in seconds, �ð� �� ������

	void Reset(); // Call before message loop.
	void Start(); // Call when unpaused.
	void Stop();  // Call when paused.
	void Tick();  // Call every frame.
};

