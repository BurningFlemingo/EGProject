#include "include/PArena.h"
#include "include/PString.h"
#include "include/PFileIO.h"
#include "include/PAssert.h"
#include "Engine/include/Logging.h"

#include <Windows.h>
#include "new"

namespace {

	constexpr uint64_t fileAccessWin32Flags[(size_t)pstd::FileAccess::COUNT]{
		0, GENERIC_READ, GENERIC_WRITE, GENERIC_READ | GENERIC_WRITE
	};

	constexpr uint64_t fileShareWin32Flags[(size_t)pstd::FileShare::COUNT]{
		0, FILE_SHARE_READ, FILE_SHARE_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE
	};

	constexpr uint64_t fileCreateWin32Flags[(size_t)pstd::FileCreate::COUNT]{
		0, CREATE_NEW, CREATE_ALWAYS, OPEN_EXISTING, OPEN_ALWAYS
	};

	using FileHandleImpl = HANDLE;
	using DllHandleImpl = HINSTANCE;
}  // namespace

pstd::FileHandle pstd::openFile(
	const char* filepath,
	const FileAccess& accessFlags,
	const FileShare& shareFlags,
	const FileCreate& createFlags
) {
	ASSERT((uint32_t)accessFlags < pstd::FileAccess::COUNT);
	ASSERT((uint32_t)shareFlags < pstd::FileShare::COUNT);
	ASSERT((uint32_t)createFlags < pstd::FileCreate::COUNT);

	uint64_t fileAccessFlags{ fileAccessWin32Flags[(size_t)accessFlags] };
	uint64_t fileShareFlags{ fileShareWin32Flags[(size_t)shareFlags] };
	uint64_t fileCreateFlags{ fileCreateWin32Flags[(size_t)createFlags] };

	HANDLE hFile{ CreateFile(
		filepath,
		fileAccessFlags,
		fileShareFlags,
		0,
		fileCreateFlags,
		FILE_ATTRIBUTE_NORMAL,
		0
	) };

	ASSERT(hFile != INVALID_HANDLE_VALUE);

	return hFile;
}

bool pstd::copyFile(const char* srcName, const char* dstName, bool replace) {
	auto res{ (bool)CopyFileA(srcName, dstName, !replace) };
	return res;
}

pstd::String pstd::getEXEPath(FixedArena* arena) {
	char exePath[MAX_PATH];
	GetModuleFileNameA(0, exePath, MAX_PATH);
	pstd::String res{ pstd::createString(arena, exePath) };
	return res;
}

pstd::DllHandle pstd::loadDll(const char* filepath) {
	HINSTANCE handle{ LoadLibraryA(filepath) };

	LOG_INFO("filename: %m\n", filepath);
	LOG_INFO("error: %u\n", (uint32_t)GetLastError());
	return handle;
}

void pstd::unloadDll(DllHandle pHandle) {
	auto handle{ (DllHandleImpl)pHandle };
	FreeLibrary(handle);
}

void* pstd::findDllFunction(DllHandle pHandle, const char* functionName) {
	auto handle{ (DllHandleImpl)pHandle };
	auto* functionPtr{ (void*)GetProcAddress(handle, functionName) };
	ASSERT(functionPtr);
	return functionPtr;
}

void pstd::closeFile(pstd::FileHandle pHandle) {
	ASSERT(pHandle);
	auto hFile{ (FileHandleImpl)pHandle };

	CloseHandle(hFile);
}

size_t pstd::getFileSize(FileHandle pHandle) {
	auto hFile{ (FileHandleImpl)pHandle };

	size_t fileSize{};
	LARGE_INTEGER win32FileSize{};
	if (GetFileSizeEx(hFile, &win32FileSize) == false) {
		ASSERT(false);
	}

	fileSize = win32FileSize.QuadPart;
	return fileSize;
}

size_t pstd::getLastFileWriteTime(const char* filename) {
	WIN32_FILE_ATTRIBUTE_DATA attribData{};
	GetFileAttributesExA(filename, GetFileExInfoStandard, &attribData);
	size_t time{ attribData.ftLastWriteTime.dwHighDateTime };
	time = time << 32;
	time |= attribData.ftLastWriteTime.dwLowDateTime;
	return time;
}

pstd::Allocation
	pstd::readFile(pstd::FixedArena* arena, pstd::FileHandle pHandle) {
	auto hFile{ (FileHandleImpl)pHandle };
	DWORD bytesRead{};
	OVERLAPPED ol{};

	size_t fileSize{ pstd::getFileSize(hFile) };

	ASSERT(fileSize < pstd::getAvaliableCount<char>(*arena));

	pstd::Allocation fileAlloc{ pstd::alloc<char>(arena, fileSize) };
	if (ReadFile(hFile, fileAlloc.block, fileSize, &bytesRead, &ol) == false) {
		ASSERT(false);
	}

	fileAlloc.size = bytesRead;
	return fileAlloc;
}
