#include "Core/PFileIO.h"

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

pstd::String pstd::readFile(pstd::Arena* pArena, const char* filePath) {
	pstd::FileHandle fileHandle{ pstd::openFile(
		filePath,
		pstd::FileAccess::read,
		pstd::FileShare::read,
		pstd::FileCreate::openExisting
	) };

	pstd::String fileString{ readFile(pArena, fileHandle) };

	pstd::closeFile(fileHandle);

	return fileString;
}
pstd::String pstd::readFile(pstd::Arena* pArena, pstd::String filePath) {
	pstd::FileHandle fileHandle{ pstd::openFile(
		pArena,
		filePath,
		pstd::FileAccess::read,
		pstd::FileShare::read,
		pstd::FileCreate::openExisting
	) };

	pstd::String fileString{ readFile(pArena, fileHandle) };

	pstd::closeFile(fileHandle);

	return fileString;
}
