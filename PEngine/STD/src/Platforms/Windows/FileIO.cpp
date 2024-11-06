#include "include/PArena.h"
#include "include/PString.h"
#include "include/PFileIO.h"
#include "include/PAssert.h"

#include <Windows.h>
#include "new"

namespace {
	struct FileHandleImpl {
		HANDLE hFile;
	};

	constexpr uint64_t fileAccessWin32Flags[(size_t)pstd::FileAccess::COUNT]{
		0, GENERIC_READ, GENERIC_WRITE, GENERIC_READ | GENERIC_WRITE
	};

	constexpr uint64_t fileShareWin32Flags[(size_t)pstd::FileShare::COUNT]{
		0, FILE_SHARE_READ, FILE_SHARE_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE
	};

	constexpr uint64_t fileCreateWin32Flags[(size_t)pstd::FileCreate::COUNT]{
		0, CREATE_NEW, CREATE_ALWAYS, OPEN_EXISTING, OPEN_ALWAYS
	};
}  // namespace

pstd::FileHandle pstd::openFile(
	FixedArena* arena,
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

	Allocation fileHandleAlloc{ pstd::arenaAlloc<FileHandleImpl>(arena) };
	pstd::FileHandle handle{ new (fileHandleAlloc.block)
								 FileHandleImpl{ .hFile = hFile } };
	return handle;
}

pstd::FileHandle pstd::openFile(
	FixedArena* arena,
	const String& filepath,
	const FileAccess& accessFlags,
	const FileShare& shareFlags,
	const FileCreate& createFlags
) {
	pstd::FileHandle handle{ pstd::openFile(
		arena,
		pstd::makeNullTerminated(arena, filepath).buffer,
		accessFlags,
		shareFlags,
		createFlags
	) };
	return handle;
}

void pstd::closeFile(const pstd::FileHandle& handle) {
	ASSERT(handle);

	FileHandleImpl* handleImpl{ (FileHandleImpl*)handle };
	CloseHandle(handleImpl->hFile);
}

size_t pstd::getFileSize(const FileHandle& handle) {
	size_t fileSize{};
	FileHandleImpl* handleImpl{ (FileHandleImpl*)handle };

	LARGE_INTEGER win32FileSize{};
	if (GetFileSizeEx(handleImpl->hFile, &win32FileSize) == false) {
		ASSERT(false);
	}

	fileSize = win32FileSize.QuadPart;
	return fileSize;
}

pstd::Allocation
	pstd::readFile(pstd::FixedArena* arena, const pstd::FileHandle& handle) {
	FileHandleImpl* handleImpl{ (FileHandleImpl*)handle };

	DWORD bytesRead{};
	OVERLAPPED ol{};

	size_t fileSize{ pstd::getFileSize(handle) };

	ASSERT(fileSize < pstd::getAvaliableCount<char>(*arena));

	pstd::Allocation fileAlloc{ pstd::arenaAlloc<char>(arena, fileSize) };
	if (ReadFile(
			handleImpl->hFile, fileAlloc.block, fileSize, &bytesRead, &ol
		) == false) {
		ASSERT(false);
	}

	fileAlloc.size = bytesRead;
	return fileAlloc;
}
