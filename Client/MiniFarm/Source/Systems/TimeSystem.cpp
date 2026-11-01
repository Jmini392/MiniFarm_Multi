#include "PCH.h"
#include "TimeSystem.h"

TimeSystem::TimeSystem()
{
	m_dayTime = 12.f;
	m_timeSpeed = DEFAULT_TIME_SPEED;
}

void TimeSystem::Update(int dt)
{
	m_dayTime += dt * m_timeSpeed;

	if (m_dayTime >= DAY_CYCLE)
	{
		m_dayTime -= DAY_CYCLE;
	}
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