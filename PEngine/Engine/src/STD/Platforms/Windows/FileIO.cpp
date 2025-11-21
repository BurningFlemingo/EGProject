#include "STD/PArena.h"
#include "STD/PString.h"
#include "STD/PFileIO.h"
#include "STD/PAssert.h"
#include "Logging.h"

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
	ASSERT(accessFlags < pstd::FileAccess::COUNT);
	ASSERT(shareFlags < pstd::FileShare::COUNT);
	ASSERT(createFlags < pstd::FileCreate::COUNT);

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

	if (hFile == INVALID_HANDLE_VALUE) {
		DWORD errorCode{ GetLastError() };
		LOG_ERROR(
			"%m could not be opened: error code %u",
			pstd::createString(filepath),
			ncast<uint32_t>(errorCode)
		);
	}

	return hFile;
}

bool pstd::copyFile(const char* dstName, const char* srcName, bool replace) {
	auto res{ (bool)CopyFileA(srcName, dstName, !replace) };
	return res;
}

// TODO: change the name of this function
pstd::String pstd::getEXEPath(Arena* pArena) {
	char exePath[MAX_PATH];
	GetModuleFileNameA(0, exePath, MAX_PATH);
	pstd::String res{ pstd::createString(pArena, exePath) };

	return res;
}

pstd::String pstd::getDllExtensionName() {
	return pstd::String("dll");
}

pstd::DllHandle pstd::loadDll(const char* filepath) {
	HINSTANCE handle{ LoadLibraryA(filepath) };

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

uint32_t pstd::getFileSize(FileHandle pHandle) {
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
	ULARGE_INTEGER time{ .LowPart = attribData.ftLastWriteTime.dwLowDateTime,
						 .HighPart =
							 attribData.ftLastWriteTime.dwHighDateTime };

	return cast<size_t>(time.QuadPart);
}

pstd::Allocation pstd::readFile(pstd::Arena* pArena, pstd::FileHandle pHandle) {
	auto hFile{ rcast<FileHandleImpl>(pHandle) };
	DWORD bytesRead{};
	OVERLAPPED ol{};

	uint32_t fileSize{ pstd::getFileSize(hFile) };

	ASSERT(fileSize < pstd::getAvailableCount<char>(*pArena));

	void* fileBuffer{ pstd::alloc(pArena, fileSize, 1) };

	if (ReadFile(hFile, rcast<void*>(fileBuffer), fileSize, &bytesRead, &ol) ==
		false) {
		ASSERT(false);
	}

	return pstd::Allocation{ .block = fileBuffer, .size = bytesRead };
}
