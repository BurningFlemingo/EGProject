#pragma once

#ifdef GAME_PROJECT
	#ifdef WINDOWS_PLATFORM
		#define GAME_API extern "C" __declspec(dllexport)
	#else
		#error platform not supported
	#endif
#else
	#define GAME_API
#endif
