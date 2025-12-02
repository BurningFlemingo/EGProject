#include "STD/PHashMap.h"

template<>
size_t pstd::hash(uint64_t val) {
	return val;
}
