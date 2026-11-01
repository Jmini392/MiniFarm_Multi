#pragma once

class TimeSystem
{
public:
	TimeSystem();

	void Update(int dt);
	
	float GetDayTime() const { return m_dayTime; }
	float GetDayProgress() const;
	bool IsDaytime() const;
	bool IsNighttime() const;
	
	void SetDayTime(float time) { m_dayTime = time; }
	void SetTimeSpeed(float speed) { m_timeSpeed = speed; }

private:
	float m_dayTime;
	float m_timeSpeed;
	
	const float DAY_START = 6.f;
	const float DAY_END = 18.f;
	const float DAY_CYCLE = 24.f;
	const float DEFAULT_TIME_SPEED = 0.00001667f;
};
