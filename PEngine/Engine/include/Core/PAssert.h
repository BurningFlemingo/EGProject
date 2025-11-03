#pragma once

#ifdef DEBUG_BUILD
	#if defined(_MSC_VER)
		#define ASSERT(expr, ...)                                \
			do {                                                 \
				if (!(expr)) {                                   \
					pstd::consoleWriteAssertionFailure(          \
						__FILE__, __LINE__, #expr, ##__VA_ARGS__ \
					);                                           \
					__debugbreak();                              \
				}                                                \
			} while (false)
	#endif
#else
	#define ASSERT(expr) \
		do {             \
		} while (false)
#endif

namespace pstd {
	bool consoleWriteAssertionFailure(
		const char* file, int line, const char* expr, const char* msg = nullptr
	);
}
