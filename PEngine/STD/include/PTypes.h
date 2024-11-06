#pragma once

static_assert(sizeof(char) == 1, "char is an incorrect size");
static_assert(sizeof(short) == 2, "short is an incorrect size");
static_assert(sizeof(int) == 4, "int is an incorrect size");
static_assert(sizeof(long long) == 8, "long long is an incorrect size");

using uint8_t = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned int;
using uint64_t = unsigned long long;

using int8_t = signed char;
using int16_t = signed short;
using int32_t = int;
using int64_t = long long;

using size_t = uint64_t;
