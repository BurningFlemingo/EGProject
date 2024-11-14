#pragma once
#include "PArena.h"
#include "PTypes.h"
#include "PString.h"

namespace Console {
	enum class LogLevel : uint32_t {
		none = 0,
		info,
		warn,
		error,

		count,
	};

	pstd::ArenaFrame getLogArena();

	void log(const LogLevel logLevel, const pstd::String& msg);

	inline void log(const pstd::String& msg) {
		log(LogLevel::none, msg);
	}

	inline void log(const LogLevel logLevel, const char* msg) {
		pstd::String msgString{ pstd::createString(msg) };
		log(logLevel, msgString);
	}

	inline void log(const char* msg) {
		log(LogLevel::none, msg);
	}

	template<typename... Args>
	void
		log(const LogLevel logLevel, const pstd::String& format, Args... args) {
		pstd::ArenaFrame logArena{ getLogArena() };
		pstd::String formattedString{
			pstd::formatString(&logArena, format, args...)
		};
		log(logLevel, formattedString);
	}

	template<typename... Args>
	void log(const LogLevel logLevel, const char* format, Args... args) {
		log(logLevel, pstd::createString(format), args...);
	}

	template<typename... Args>
	void log(const pstd::String& format, Args... args) {
		log(LogLevel::none, format, args...);
	}

	template<typename... Args>
	void log(const char* format, Args... args) {
		log(LogLevel::none, format, args...);
	}
}  // namespace Console

using Console::LogLevel;

#ifdef LOG_LEVEL_INFO
	#define LOG_INFO(format, ...)                \
		{                                        \
			Console::log(                        \
				Console::LogLevel::info,         \
				"[%m:%i] ",                      \
				pstd::getFileName(__FILE__),     \
				__LINE__                         \
			);                                   \
			Console::log(format, ##__VA_ARGS__); \
		}
#else
	#define LOG_INFO(format, ...)
#endif

#ifdef LOG_LEVEL_WARN
	#define LOG_WARN(format, ...)                \
		{                                        \
			Console::log(                        \
				Console::LogLevel::warn,         \
				"[%m:%i] ",                      \
				pstd::getFileName(__FILE__),     \
				__LINE__                         \
			);                                   \
			Console::log(format, ##__VA_ARGS__); \
		}
#else
	#define LOG_WARN(format, ...)
#endif

#ifdef LOG_LEVEL_ERROR
	#define LOG_ERROR(format, ...)               \
		{                                        \
			Console::log(                        \
				Console::LogLevel::error,        \
				"[%m:%i] ",                      \
				pstd::getFileName(__FILE__),     \
				__LINE__                         \
			);                                   \
			Console::log(format, ##__VA_ARGS__); \
		}
#else
	#define LOG_ERROR(format, ...)
#endif
