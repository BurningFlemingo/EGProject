#pragma once

#ifdef DEBUG_BUILD
	#if defined(_MSC_VER)
		#define ASSERT(expr)        \
			do {                    \
				if (!(expr)) {      \
					__debugbreak(); \
				}                   \
			} while (false)
	#endif
#else
	#define ASSERT(expr) \
		do {             \
		} while (false)
#endif
