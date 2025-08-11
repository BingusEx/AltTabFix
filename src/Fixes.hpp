#pragma once

namespace AltTabFix {
	constexpr RE::InputEvent* const dummy[] = { nullptr };
	static volatile inline int8_t BlockFrameCount = 0;
	void ResetInput();
}