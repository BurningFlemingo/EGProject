#include "STD/PFileIO.h"
#include "Logging.h"

pstd::String pstd::makeExeDirectoryPath(pstd::Arena* pPersistArena) {
	pstd::String exeString{ pstd::getEXEPath(pPersistArena) };

	uint32_t seperatorIndex{};
	bool seperatorFound{ pstd::substringMatchBackward(
		exeString, pstd::createString("/"), &seperatorIndex
	) };

	if (!seperatorFound) {
		pstd::substringMatchBackward(
			exeString, pstd::createString("\\"), &seperatorIndex
		);
	}
	exeString.size = seperatorIndex + 1;
	return exeString;
}

pstd::Allocation pstd::readFile(pstd::Arena* pArena, const char* filePath) {
	pstd::FileHandle fileHandle{ pstd::openFile(
		filePath,
		pstd::FileAccess::read,
		pstd::FileAccess::read,
		pstd::FileCreate::openExisting
	) };

	pstd::Allocation fileAllocation{ readFile(pArena, fileHandle) };

	pstd::closeFile(fileHandle);

	return fileAllocation;
}
pstd::Allocation pstd::readFile(pstd::Arena* pArena, pstd::String filePath) {
	pstd::FileHandle fileHandle{ pstd::openFile(
		pArena,
		filePath,
		pstd::FileAccess::read,
		pstd::FileAccess::read,
		pstd::FileCreate::openExisting
	) };

	pstd::Allocation fileAllocation{ readFile(pArena, fileHandle) };

	pstd::closeFile(fileHandle);

	return fileAllocation;
}
