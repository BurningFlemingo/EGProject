#pragma once

namespace pstd {
	template<typename T>
	struct optional {
		T val;
		bool exists;
	};
}  // namespace pstd
