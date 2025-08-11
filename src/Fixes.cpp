#include "Fixes.hpp"

namespace AltTabFix {

	void ResetInput() {

		if (const auto& InputManger = RE::BSInputDeviceManager::GetSingleton()) {

			if (const auto& Device = InputManger->GetKeyboard()) {
				Device->Reset();
				Device->Process(0);

				//Also Explicitly send  KeyUp Events
				INPUT input {};
				input.type = INPUT_KEYBOARD;
				input.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_EXTENDEDKEY;

				constexpr int vks[] = {
					VK_LSHIFT, VK_RSHIFT,
					VK_LCONTROL, VK_RCONTROL,
					VK_LMENU, VK_RMENU,
					VK_TAB
				};

				for (int vk : vks) {
					input.ki.wVk = vk;
					SendInput(1, &input, sizeof(input));
				}

				// Flush message queue of Alt keys
				MSG msg;
				while (PeekMessage(&msg, nullptr, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)) {
					if (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) {
						// discard all wmessage keys
					}
				}
			}
		}

		if (auto evtq = BSInputEventQueue::GetSingleton()) {
			evtq->ClearInputQueue();

			for (auto& evt : evtq->GetRuntimeData().buttonEvents) {
				evt.value = 0.0f;
				evt.heldDownSecs = 0.0f;
				evt.idCode = 0;
				evt.userEvent = "";
				evt.next = nullptr;
			}

			for (auto& evt : evtq->GetRuntimeData().charEvents) {
				evt.keyCode = 0;
				evt.next = nullptr;
			}
		}
	}
}