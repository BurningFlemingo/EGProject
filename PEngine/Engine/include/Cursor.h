#pragma once

namespace Platform {
	struct State;

	void captureCursor(State* pState);
	void releaseCursor(State* pState);

	void hideCursor(State* pState);
	void showCursor(State* pState);
}  // namespace Platform
