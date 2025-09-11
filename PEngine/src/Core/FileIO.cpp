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
