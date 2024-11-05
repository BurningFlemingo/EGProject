#pragma once

#ifdef CRT_PROJECT
	#ifdef WINDOWS_PLATFORM
		#define CRT_API __declspec(dllexport)
	#else
	// #error platform not supported
	#endif
#else
	#ifdef WINDOWS_PLATFORM
		#define CRT_API __declspec(dllimport)
	#else
	// #error platform not supported
	#endif
#endif
