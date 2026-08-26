#pragma once
#include "Util/HookUtil.hpp"

namespace Hooks::FocusTheft {

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

	struct BSDInput_GetDeviceState {

		static void __thiscall thunk(RE::BSDirectInputManager* a_mgr, REX::W32::IDirectInputDevice8A* a_device, std::uint32_t a_size, void* a_outData) {
			
			if (BSGraphics::Renderer* renderer = RE::BSGraphics::Renderer::GetSingleton()) {
				if (BSGraphics::RendererWindow* curWindow = renderer->GetCurrentRenderWindow()) {
					if (REX::W32::HWND RexHwnd = curWindow->hWnd) {
						if (HWND hwnd = reinterpret_cast<HWND>(RexHwnd)) {
							if (GetForegroundWindow() != hwnd) {
								ZeroMemory(a_outData, a_size);
								return;
							}
						}
					}
				}
			}
			func(a_mgr, a_device, a_size, a_outData);
		}
		FUNCTYPE_DETOUR func;
	};

	struct BSDinput_GetDeviceData {

		static void __thiscall thunk(RE::BSDirectInputManager* a_mgr, REX::W32::IDirectInputDevice8A* a_device, std::uint32_t* a_dataSize, REX::W32::DIDEVICEOBJECTDATA** a_outData) {


			if (BSGraphics::Renderer* renderer = RE::BSGraphics::Renderer::GetSingleton()) {
				if (BSGraphics::RendererWindow* curWindow = renderer->GetCurrentRenderWindow()) {
					if (REX::W32::HWND RexHwnd = curWindow->hWnd) {
						if (HWND hwnd = reinterpret_cast<HWND>(RexHwnd)) {
							//Zero out devdata if not focused.
							if (GetForegroundWindow() != hwnd) {
								if (a_dataSize) *a_dataSize = 0;
								return;
							}
						}
					}
				}
			}
			func(a_mgr, a_device, a_dataSize, a_outData);
		}

		FUNCTYPE_DETOUR func;

	};

	inline void Install() {

		logger::info("Installing Anti-Focus Steal Hooks");

		//Steal Focus on window create fix
		Hooks::stl::write_call<Win32_SetForeGroundWindow, 6>(REL::RelocationID(75591, 77226, NULL), REL::VariantOffset(0x195, 0x25e, NULL));
		Hooks::stl::write_call<Win32_ShowWindow, 6>(REL::RelocationID(75591, 77226, NULL), REL::VariantOffset(0x184, 0x24d, NULL));
		Hooks::stl::write_call<Win32_SetFocus, 6>(REL::RelocationID(75591, 77226, NULL), REL::VariantOffset(0x1a6, 0x26f, NULL));

		//It appears that changing the focus state flags on startup breaks input filtering somehow, causing the game to read input while its not in focus.
		Hooks::stl::write_detour<BSDInput_GetDeviceState>(REL::RelocationID(67375, 68677, NULL));
		Hooks::stl::write_detour<BSDinput_GetDeviceData>(REL::RelocationID(67376, 68678, NULL));

	}

}