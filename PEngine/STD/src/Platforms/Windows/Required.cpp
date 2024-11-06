#include "include/PAssert.h"

extern "C" {
int _fltused{};
}

extern "C" {
void *__cdecl memset(void *dst, int val, size_t size);
void *__cdecl memcpy(void *dst, const void *src, size_t size);
int __cdecl memcmp(const void *buf1, const void *buf2, size_t size);

#pragma function(memset)
void *memset(void *dst, int val, size_t size) {
	auto block{ (char *)dst };
	for (int i{}; i < size; i++) {
		*block = (char)val;
		block++;
	}

	return dst;
}

#pragma function(memcpy)
void *memcpy(void *dst, const void *src, size_t size) {
	ASSERT(dst);
	ASSERT(src);

	auto dstBlock{ (char *)dst };
	auto srcBlock = (const char *)src;

	while (size--) {
		*dstBlock = *srcBlock;
		dstBlock++;
		srcBlock++;
	}
	return dst;
}

#pragma function(memcmp)
int memcmp(const void *buf1, const void *buf2, size_t size) {
	ASSERT(buf1);
	ASSERT(buf2);

	auto *block1{ (const unsigned char *)buf1 };
	auto *block2{ (const unsigned char *)buf2 };
	for (size_t i{}; i < size; i++) {
		if (block1[i] != block2[i]) {
			return block1[i] - block2[i];
		}
	}

	return 0;
}

int _wcsicmp(const wchar_t *buf1, const wchar_t *buf2) {
	wchar_t str1{ *buf1 };
	wchar_t str2{ *buf2 };
	while (str1) {
		if (str1 >= 'A' && str2 <= 'Z') {
			str1 += 'a' - 'A';
		}
		if (str2 >= 'A' && str2 <= 'Z') {
			str2 += 'a' - 'A';
		}

		if (str1 != str2) {
			return str1 - str2;
		}

		buf1++;
		buf2++;
		str1 = *buf1;
		str2 = *buf2;
	}
	return 0;
}
}
