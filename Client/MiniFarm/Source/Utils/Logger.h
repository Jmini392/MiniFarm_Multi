#pragma once
#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdarg>

#define LOG(msg, ...) Logger::GetInstance().LogMessage(LogType::Log, __FUNCTION__, msg,##__VA_ARGS__)
#ifdef _DEBUG
#define LOG_D(msg, ...) Logger::GetInstance().LogMessage(LogType::Debug,  __FUNCTION__, msg, ##__VA_ARGS__)
#else
#define LOG_D(msg, ...)
#endif
#define LOG_E(msg, ...) Logger::GetInstance().LogMessage(LogType::Error, __FUNCTION__,  msg, ##__VA_ARGS__)

enum LogType
{
	Log,
	Debug,
	Error,
};

class Logger
{
public:
	static Logger& GetInstance()
	{
		static Logger instance;
		return instance;
	}

	void LogMessage(LogType type, const char* functionName, const char* fmt, ...)
	{
		std::lock_guard<std::mutex> lock(_logLock);

		char buffer[512];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);

		std::string level = LevelToString(type);
		std::string time = GetCurrentTime();

		std::cout << time << level << "[" << functionName << "] " << buffer << '\n';
	}

private:
	std::string LevelToString(LogType type)
	{
		switch (type)
		{
		case LogType::Error: return "[Error] ";
		case LogType::Debug: return "[Debug] ";
		case LogType::Log: return "[Log] ";
		default: return "[Unknown] ";
		}
	}

	std::string GetCurrentTime()
	{
		auto now = std::chrono::system_clock::now();
		std::time_t now_c = std::chrono::system_clock::to_time_t(now);
		struct tm timeInfo;
		localtime_s(&timeInfo, &now_c);

		std::ostringstream oss;
		oss << std::put_time(&timeInfo, "[%H:%M:%S]");
		return oss.str();
	}

	std::mutex _logLock;
};
