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
		ArenaFrame&& frame,
		const String& filepath,
		const FileAccess& accessFlags,
		const FileShare& shareFlags,
		const FileCreate& createFlags
	) {
		return pstd::openFile(
			pstd::makeNullTerminated({ frame.pArena, frame.state }, filepath)
				.buffer,
			accessFlags,
			shareFlags,
			createFlags
		);
	}

	bool copyFile(const char* srcName, const char* dstName, bool replace);

	String getEXEPath(ArenaFrame&& frame);	// includes the exe name

	DllHandle loadDll(const char* filepath);
	void unloadDll(DllHandle handle);
	void* findDllFunction(DllHandle handle, const char* functionName);

	void closeFile(FileHandle handle);

	size_t getFileSize(FileHandle handle);
	size_t getLastFileWriteTime(const char*);

	Allocation readFile(ArenaFrame&& frame, FileHandle handle);
}  // namespace pstd
