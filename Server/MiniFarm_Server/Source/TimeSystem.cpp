#include "TimeSystem.h"
#include <cmath>

TimeSystem::TimeSystem()
{
	m_dayTime = 12.f;
	m_timeSpeed = DEFAULT_TIME_SPEED;
}

void TimeSystem::Update(int dt)
{
	m_dayTime += dt * m_timeSpeed;

	// 한 번에 24시간 이상 증가해도 하루 범위로 변경
	m_dayTime = std::fmod(m_dayTime, DAY_CYCLE);
}

float TimeSystem::GetDayProgress() const
{
	if (m_dayTime >= DAY_START && m_dayTime < DAY_END)
	{
		return (m_dayTime - DAY_START) / (DAY_END - DAY_START);
	}
	return 0.f;
}

bool TimeSystem::IsDaytime() const
{
	return m_dayTime >= DAY_START && m_dayTime < DAY_END;
}

bool TimeSystem::IsNighttime() const
{
	return !IsDaytime();
}