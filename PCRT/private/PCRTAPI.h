#pragma once

#ifdef PCRT_PROJECT
	#ifdef WINDOWS_PLATFORM
		#define PCRT_API __declspec(dllexport)
	#else
		#error platform not supported
	#endif
#elif defined(PCRT_BASE_PROJECT)
	#ifdef WINDOWS_PLATFORM
		#define PCRT_API __declspec(dllimport)
	#else
		#error platform not supported
	#endif
#else
	#error internal header included by an external project
#endif
