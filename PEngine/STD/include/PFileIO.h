#pragma once
#include "PArena.h"
#include "PString.h"

namespace pstd {
	using FileHandle = void*;

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
		FixedArena* arena,
		const char* filepath,
		const FileAccess& accessFlags,
		const FileShare& shareFlags,
		const FileCreate& createFlags
	);

	FileHandle openFile(
		FixedArena* arena,
		const String& filepath,
		const FileAccess& accessFlags,
		const FileShare& shareFlags,
		const FileCreate& createFlags
	);

	void closeFile(const FileHandle& handle);

	size_t getFileSize(const FileHandle& handle);

	Allocation readFile(FixedArena* arena, const FileHandle& handle);
}  // namespace pstd
