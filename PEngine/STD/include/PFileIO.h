#pragma once
#include "PArena.h"
#include "PString.h"

namespace pstd {
	using FileHandle = void*;
	using DllHandle = void*;

	enum class FileAccess : uint32_t { none, read, write, readwrite, COUNT };
	enum class FileShare : uint32_t { none, read, write, readwrite, COUNT };
	enum class FileCreate : uint32_t {
		none,
		createNew,
		createAlways,
		openExisting,
		openAlways,
		COUNT
	};

	FileHandle openFile(
		const char* filepath,
		const FileAccess& accessFlags,
		const FileShare& shareFlags,
		const FileCreate& createFlags
	);

	inline FileHandle openFile(
		FixedArena arena,
		const String& filepath,
		const FileAccess& accessFlags,
		const FileShare& shareFlags,
		const FileCreate& createFlags
	) {
		return pstd::openFile(
			pstd::makeNullTerminated(&arena, filepath).buffer,
			accessFlags,
			shareFlags,
			createFlags
		);
	}

	bool copyFile(const char* srcName, const char* dstName, bool replace);

	String getEXEPath(FixedArena* arena);  // includes the exe name

	DllHandle loadDll(const char* filepath);
	void unloadDll(DllHandle handle);
	void* findDllFunction(DllHandle handle, const char* functionName);

	void closeFile(FileHandle handle);

	size_t getFileSize(FileHandle handle);
	size_t getLastFileWriteTime(const char*);

	Allocation readFile(FixedArena* arena, FileHandle handle);
}  // namespace pstd
