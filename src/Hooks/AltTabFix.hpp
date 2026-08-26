#pragma once
#include "Util/HookUtil.hpp"

namespace Hooks::AltTabFix {

	constexpr RE::InputEvent* const dummy[] = { nullptr };
	static volatile inline int8_t BlockFrameCount = 0;
	static inline HWND windowHandle = nullptr;

	//Main::Update has an async key check that immediatly exits out of the function
	//This was probably bethesda's attempt at preventing any stray input parsing.
	//I disabled it with the assumption that it will interfere with the "postfix" cleanup i do
	//idk if it actually makes a difference though.
	//This is actually important As the early return messes with the input prevention.
	//Does this mean input is checked/dispatched on a different thread?
	inline void RemoveAsyncKeyCheck() {

		logger::trace("Removing Main::Update Key check");
		//1.7.99 = 0x140658610 - 0x140658671 = 0x61, Unchanged luckily
		REL::Relocation<std::uintptr_t> jmp{ VariantID(35565, 36564, NULL), VariantOffset(0x46, 0x61, NULL) };

		// disable jmp.
		REL::safe_fill(jmp.address(), 0x90, 6);
	}

	inline void ResetInput() {

		if (const auto& InputManger = RE::BSInputDeviceManager::GetSingleton()) {

			if (const auto& Device = InputManger->GetKeyboard()) {
				Device->ClearInputState(); //memset(0) the input buffer
				Device->Poll(0);

				//This messes with the alt tab menu in windows, besides its not windows that has stuck keys
				//so this is useless here.
				/*
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
				}*/

				// Flush window message queue of all keys
				MSG msg;
				while (PeekMessage(&msg, nullptr, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE));
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

			windowHandle = a_hwnd;
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
		const bool Pre17 = Module::get().version() <= RUNTIME_1_6_1179;
		logger::info("Module Version: {}", Module::get().version().string());

		//ALT Tab Fix
		Hooks::stl::write_call<Win32_RegisterClassA, 6>(REL::RelocationID(75591, 77226, NULL), REL::VariantOffset(0x8E, 0x15C, NULL));
		Hooks::stl::write_call<Input_DispatchEvent, 5>(REL::RelocationID(67315, 68617, NULL), REL::VariantOffset(0x7B, 0x7B, NULL));
		Hooks::stl::write_call<Main_Update_Post, 5>(REL::RelocationID(35565, 36564, NULL), REL::VariantOffset(0x748, Pre17 ? 0xC26 : 0xC38, NULL));

		RemoveAsyncKeyCheck();
		
	}
}