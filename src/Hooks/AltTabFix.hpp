#pragma once
#include "Util/HookUtil.hpp"

namespace Hooks::AltTabFix {

	constexpr RE::InputEvent* const dummy[] = { nullptr };
	static volatile inline int8_t BlockFrameCount = 0;

	inline void RemoveAsyncKeyCheck() {

		logger::trace("Removing Main::Update Key check");
		REL::Relocation<std::uintptr_t> jmp{ VariantID(35565, 36564, NULL), VariantOffset(0x46, 0x61, NULL) };

		// disable jmp.
		REL::safe_fill(jmp.address(), 0x90, 6);
	}

	inline void ResetInput() {

		if (const auto& InputManger = RE::BSInputDeviceManager::GetSingleton()) {

			if (const auto& Device = InputManger->GetKeyboard()) {
				Device->Reset();
				Device->Process(0);

				//Also Explicitly send  KeyUp Events
				INPUT input{};
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

	struct Main_Update_Post {

		static void thunk(RE::Main* a_this, float a_deltaTime) {

			func(a_this, a_deltaTime);

			if (AltTabFix::BlockFrameCount > 0) {
				AltTabFix::BlockFrameCount--;
				AltTabFix::ResetInput();
			}

		}

		FUNCTYPE_CALL func;
	};

	struct WndProcHandler {

		static LRESULT thunk(HWND a_hwnd, UINT a_msg, WPARAM a_wParam, LPARAM a_lParam) {

			switch (a_msg) {

				case WM_ACTIVATEAPP:
				case WM_ENABLE:
				case WM_SHOWWINDOW:
				{

					ResetInput();
					BlockFrameCount = Config::ConfigManager::AltTabFix.iFramesToBlock;

					break;

				}

				default:{};

			}

			return func(a_hwnd, a_msg, a_wParam, a_lParam);
		}

		FUNCTYPE_CALL func;
	};

	struct Win32_RegisterClassA {

		static WORD thunk(WNDCLASSA* a_wndClass) {

			WndProcHandler::func = reinterpret_cast<uintptr_t>(a_wndClass->lpfnWndProc);
			a_wndClass->lpfnWndProc = &WndProcHandler::thunk;

			return func(a_wndClass);
		}

		FUNCTYPE_CALL func;
	};

	struct Input_DispatchEvent {

		static void thunk(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent** a_events) {

			//Pass a dummy inputevent LL for the num of frames specified.
			if (AltTabFix::BlockFrameCount > 0) {
				a_events = const_cast<RE::InputEvent**>(AltTabFix::dummy);
			}

			func(a_dispatcher, a_events);

		}

		FUNCTYPE_CALL func;
	};

	inline void Install() {

		logger::info("Installing AltTabFix Hooks");

		//ALT Tab Fix
		Hooks::stl::write_call<Win32_RegisterClassA, 6>(REL::RelocationID(75591, 77226, NULL), REL::VariantOffset(0x8E, 0x15C, NULL));
		Hooks::stl::write_call<Main_Update_Post, 5>(REL::RelocationID(35565, 36564, NULL), REL::VariantOffset(0x748, 0xC26, NULL));
		Hooks::stl::write_call<Input_DispatchEvent, 5>(REL::RelocationID(67315, 68617, NULL), REL::VariantOffset(0x7B, 0x7B, NULL));

		RemoveAsyncKeyCheck();

	}

}