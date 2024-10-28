#pragma once
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

	template<typename T>
	void
		log(const pstd::String& format,
			const T val,
			const LogLevel logLevel = LogLevel::none);

	template<typename T>
	void
		log(const char* format,
			const T val,
			const LogLevel logLevel = LogLevel::none) {
		pstd::String formatString{ pstd::createString(format) };
		log(formatString, val, logLevel);
	}

	void log(const pstd::String& msg, const LogLevel logLevel = LogLevel::none);
	void log(const char* msg, const LogLevel logLevel = LogLevel::none);
}  // namespace Console

using Console::LogLevel;

#ifdef LOG_LEVEL_INFO
	#define LOG_INFO(format, ...)                      \
		{                                              \
			Console::log("[INFO] [");                  \
			Console::log(pstd::getFileName(__FILE__)); \
			Console::log(":%i", __LINE__);             \
			Console::log("] ");                        \
			Console::log(format, __VA_ARGS__);         \
		}
#else
	#define LOG_INFO(format, ...)
#endif

#ifdef LOG_LEVEL_WARN
	#define LOG_WARN(format, ...)                      \
		{                                              \
			Console::log("[WARN] [");                  \
			Console::log(pstd::getFileName(__FILE__)); \
			Console::log(":%i", __LINE__);             \
			Console::log("] ");                        \
			Console::log(format, __VA_ARGS__);         \
		}
#else
	#define LOG_WARN(format, ...)
#endif

#ifdef LOG_LEVEL_ERROR
	#define LOG_ERROR(format, ...)                     \
		{                                              \
			Console::log("[ERROR] [");                 \
			Console::log(pstd::getFileName(__FILE__)); \
			Console::log(":%i", __LINE__);             \
			Console::log("] ");                        \
			Console::log(format, __VA_ARGS__);         \
		}
#else
	#define LOG_ERROR(format, ...)
#endif
