#pragma once
#include "HookUtil.hpp"
#include "Fixes.hpp"

namespace Hook {

	struct MainUpdatePost {

		static void thunk(RE::Main* a_this, float a_deltaTime) {

			func(a_this, a_deltaTime);

			if (AltTabFix::BlockFrameCount > 0) {
				AltTabFix::BlockFrameCount--;
				AltTabFix::ResetInput();
			}

		}

		FUNCTYPE_CALL func;
	};

	struct MainUpdatePre {

		static void thunk(int64_t unk_1) {
			func(unk_1);
		}

		FUNCTYPE_CALL func;
	};

	struct WndProcHandler {

		static LRESULT thunk(HWND a_hwnd, UINT a_msg, WPARAM a_wParam, LPARAM a_lParam) {

			//Fires On Focus Loss & Gain
			if (a_msg == WM_ACTIVATEAPP || a_msg == WM_ENABLE || a_msg == WM_SHOWWINDOW) {
				AltTabFix::ResetInput();
				AltTabFix::BlockFrameCount = 2;
			}

			return func(a_hwnd, a_msg, a_wParam, a_lParam);
		}

		FUNCTYPE_CALL func;
	};

	struct RegisterClassA {

		static WORD thunk(WNDCLASSA* a_wndClass) {

			WndProcHandler::func = reinterpret_cast<uintptr_t>(a_wndClass->lpfnWndProc);
			a_wndClass->lpfnWndProc = &WndProcHandler::thunk;

			return func(a_wndClass);
		}

		FUNCTYPE_CALL func;
	};

	struct InputDispatchEvent {

		static void thunk(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent** a_events) {

			if (AltTabFix::BlockFrameCount > 0) {
				a_events = const_cast<RE::InputEvent**>(AltTabFix::dummy);
			}

			func(a_dispatcher, a_events);

		}

		FUNCTYPE_CALL func;
	};


	inline void RemoveAsyncKeyCheck() {
		REL::Relocation<std::uintptr_t> JumpLoc{ VariantID(35565, 36564, NULL), VariantOffset(0x46, 0x61, NULL) };
		//Jump Over AsyncKeyCheck
		SKSE::GetTrampoline().write_branch<6>(JumpLoc.address(), JumpLoc.address() + 0x06);
	}

	inline void Install() {

		auto& Trampoline = SKSE::GetTrampoline();
		Trampoline.create(64);

		Hooks::stl::write_call<RegisterClassA, 6>(REL::VariantID(75591, 77226, NULL), REL::VariantOffset(0x8E, 0x15C, NULL));
		//Hooks::stl::write_call<MainUpdatePre, 5>(REL::VariantID(35565, 36564, NULL), REL::VariantOffset(0x1E, 0x3E, NULL));
		Hooks::stl::write_call<MainUpdatePost, 5>(REL::VariantID(35565, 36564, NULL), REL::VariantOffset(0x748, 0xC26, NULL));
		Hooks::stl::write_call<InputDispatchEvent, 5>(REL::VariantID(67315, 68617, NULL), REL::VariantOffset(0x7B, 0x7B, NULL));
		RemoveAsyncKeyCheck();
	}

}