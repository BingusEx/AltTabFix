#pragma once
#include "HookUtil.hpp"
#include "Fixes.hpp"

#include <windows.h>
#include <dinput.h>

namespace Hook {

	
	inline HWND GameWindow = nullptr;

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

	struct MainUpdatePre {

		static void thunk(int64_t unk_1) {
			func(unk_1);
		}

		FUNCTYPE_CALL func;
	};

	struct WndProcHandler {

		static LRESULT thunk(HWND a_hwnd, UINT a_msg, WPARAM a_wParam, LPARAM a_lParam) {

			switch (a_msg) {

				// "Fixes" The game preventing usage of the snipping tool shortcut if it currently has kb focus.
				case WM_SYSKEYDOWN:
				{
					const bool winDown = (GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000);
					const bool shiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x8000);

					if (winDown && shiftDown && a_wParam == 'S') {

						AltTabFix::ResetInput();
						AltTabFix::BlockFrameCount = 2;

						// Directly launch Windows Snip & Sketch
						ShellExecuteW(0, L"open", L"ms-screenclip:", NULL, NULL, SW_SHOWNORMAL);

						return 0;
					}

					break;

				}

				case WM_ACTIVATEAPP:
				case WM_ENABLE:
				case WM_SHOWWINDOW:
				{

					AltTabFix::ResetInput();
					AltTabFix::BlockFrameCount = 2;

					break;

				}

				default: {};

			}

			//Just incase the hwnd changes during runtime.
			GameWindow = a_hwnd;

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

	struct Win32_SetForeGroundWindow {

		static bool __stdcall thunk(HWND a_hwnd) {
			logger::trace("SetForeGroundWindow Proxy Call");
			return true;
		}

		FUNCTYPE_CALL func;
	};

	struct Win32_ShowWindow {

		static bool __stdcall thunk(HWND a_hwnd, int a_cmd) {
			//Replace ShowWindow (a_cmd = 5) with (a_cmd = 4)
			logger::trace("ShowWindow Proxy Call");
			return func(a_hwnd, SW_SHOWNOACTIVATE);
		}

		FUNCTYPE_CALL func;
	};

	struct Win32_SetFocus {

		static HWND __stdcall thunk(HWND a_hwnd) {
			//Stub
			logger::trace("SetFocus Proxy Call");
			return a_hwnd;
		}

		FUNCTYPE_CALL func;
	};

	inline void RemoveAsyncKeyCheck() {

		logger::trace("Removing Main::Update Key check");
		REL::Relocation<std::uintptr_t> jmp{ VariantID(35565, 36564, NULL), VariantOffset(0x46, 0x61, NULL) };

		// disable jmp.
		REL::safe_fill(jmp.address(), 0x90, 6);
	}

	struct BSDInput_GetDeviceState {

		static void __thiscall thunk(RE::BSDirectInputManager* a_mgr, REX::W32::IDirectInputDevice8A* a_device, std::uint32_t a_size, void* a_outData) {

			//Zero out data if not focused.
			if (GetForegroundWindow() != GameWindow) {
				ZeroMemory(a_outData, a_size);
				return;
			}

			func(a_mgr, a_device, a_size, a_outData);

		}

		FUNCTYPE_DETOUR func;

	};

	struct BSDinput_GetDeviceData {

		static void __thiscall thunk(RE::BSDirectInputManager* a_mgr, REX::W32::IDirectInputDevice8A* a_device, std::uint32_t* a_dataSize, REX::W32::DIDEVICEOBJECTDATA** a_outData) {

			//Zero out devdata if not focused.
			if (GetForegroundWindow() != GameWindow) {
				if (a_dataSize) *a_dataSize = 0;
				return;
			}

			func(a_mgr, a_device, a_dataSize, a_outData);

		}

		FUNCTYPE_DETOUR func;

	};

	inline void Install() {

		auto& Trampoline = SKSE::GetTrampoline();
		Trampoline.create(96);

		logger::info("Installing Alt-Tab Fix Hooks");

		//ALT Tab Fix
		Hooks::stl::write_call<Win32_RegisterClassA, 6>(REL::RelocationID(75591, 77226, NULL), REL::VariantOffset(0x8E, 0x15C, NULL));
		Hooks::stl::write_call<Main_Update_Post, 5>(REL::RelocationID(35565, 36564, NULL), REL::VariantOffset(0x748, 0xC26, NULL));
		Hooks::stl::write_call<Input_DispatchEvent, 5>(REL::RelocationID(67315, 68617, NULL), REL::VariantOffset(0x7B, 0x7B, NULL));

		logger::info("Installing Anti-Focus Steal Hooks");

		//Steal Focus on window create fix
		Hooks::stl::write_call<Win32_SetForeGroundWindow, 6>(REL::RelocationID(75591, 77226, NULL), REL::VariantOffset(0x195, 0x25e, NULL));
		Hooks::stl::write_call<Win32_ShowWindow, 6>(REL::RelocationID(75591, 77226, NULL), REL::VariantOffset(0x184, 0x24d, NULL));
		Hooks::stl::write_call<Win32_SetFocus, 6>(REL::RelocationID(75591, 77226, NULL), REL::VariantOffset(0x1a6, 0x26f, NULL));
		Hooks::stl::write_detour<BSDInput_GetDeviceState>(REL::RelocationID(67375, 68677, NULL));
		Hooks::stl::write_detour<BSDinput_GetDeviceData>(REL::RelocationID(67376, 68678, NULL));

		RemoveAsyncKeyCheck();

	}

}