#pragma once

extern "C" {
void* memset(void* dst, int val, size_t size);
void* memcpy(void* dst, const void* src, size_t size);
int memcmp(const void* buf1, const void* buf2, size_t size);
int _wcsicmp(const wchar_t* buf1, const wchar_t* buf2);
}
