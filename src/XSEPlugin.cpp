
namespace {

	namespace stl {
		using namespace SKSE::stl;

		template <class T, std::size_t Size = 5>
		void write_thunk_call(std::uintptr_t a_src) {
			SKSE::AllocTrampoline(14);
			auto& trampoline = SKSE::GetTrampoline();
			if (Size == 6) {
				T::func = *(uintptr_t*)trampoline.write_call<6>(a_src, T::thunk);
			}
			else {
				T::func = trampoline.write_call<Size>(a_src, T::thunk);
			}
		}
	}

	volatile bool focusState_Changed = false;

	void ResetInput() {

		if (const auto& InputManger = RE::BSInputDeviceManager::GetSingleton()) {

			if (const auto& Device = InputManger->GetKeyboard()) {

				logger::trace("curState Addr: {}", static_cast<void*>(&Device->curState));
				//Reset Keyboard CurState + PrevState
				std::memset(&Device->curState, 0x0, 0x200);

				//Also Explicitly send a KeyUp Event, Probably Unneeded.
				//INPUT input {};
				//input.type = INPUT_KEYBOARD;
				//input.ki.dwFlags = KEYEVENTF_KEYUP;

				//constexpr int vks[] = {
				//	VK_LSHIFT, VK_RSHIFT,
				//	VK_LCONTROL, VK_RCONTROL,
				//	VK_LMENU, VK_RMENU, //Alt
				//	VK_TAB
				//};

				//for (int vk : vks) {
				//	input.ki.wVk = vk;
				//	SendInput(1, &input, sizeof(input));
				//}
			}
		}
	}

	struct Hook_MainUpdate {

		static void thunk(RE::Main* a_this, float a_deltaTime) {

			func(a_this, a_deltaTime);

			if (focusState_Changed) {
				focusState_Changed = false;
				ResetInput();
			}

		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct Hook_WndProcHandler {

		static LRESULT thunk(HWND a_hwnd, UINT a_msg, WPARAM a_wParam, LPARAM a_lParam) {

			//Fires On Focus Loss & Gain
			if (a_msg == WM_ACTIVATEAPP) {
				focusState_Changed = true;
			}

			return func(a_hwnd, a_msg, a_wParam, a_lParam);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct Hook_RegisterClassA {

		static WORD thunk(WNDCLASSA* a_wndClass) {

			Hook_WndProcHandler::func = reinterpret_cast<uintptr_t>(a_wndClass->lpfnWndProc);
			a_wndClass->lpfnWndProc = &Hook_WndProcHandler::thunk;

			return func(a_wndClass);
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct Hook_InputDispatchEvent {

		static void thunk(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent** a_events) {

			if (focusState_Changed) {
				//a_events = { nullptr };
				constexpr RE::InputEvent* const dummy[] = { nullptr };
				a_events = const_cast<RE::InputEvent**>(dummy);
				//func(a_dispatcher, const_cast<RE::InputEvent**>(dummy));
			}

			func(a_dispatcher, a_events);

		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	//Patch GetAsync Key Check for ALT+Tab in Main update
	//Probably not needed.
	void RemoveCheck() {
		constexpr auto Addr = REL::VariantID(35565, 36564, 0);
		constexpr auto Offs = REL::VariantOffset(0x46, 0x61, 0);

		REL::Relocation<std::uintptr_t> JumpLoc{ Addr, Offs };
		SKSE::AllocTrampoline(8);
		SKSE::GetTrampoline().write_branch<6>(
			JumpLoc.address(),
			JumpLoc.address() + 0x06
		);
		

		//if (REL::Module::IsSE()) {

		//	//1405b3036 0f 85 0d JNZ LAB_1405b3749 07 00 00
		//	constexpr std::uintptr_t kJnzOffset = 0x46;

		//	REL::Relocation<std::uintptr_t> jnzLoc{ REL::ID(35565), kJnzOffset };
		//	SKSE::AllocTrampoline(8);
		//	SKSE::GetTrampoline().write_branch<6>(
		//		jnzLoc.address(),
		//		jnzLoc.address() + 0x06
		//	);
		//}

		////Untested, Both JS and JNZ are 6 byte instructions so it should be fine?
		//if (REL::Module::IsAE()) {

		//	// 140645f01 0f 88 d5 JS LAB_140646adc 0b 00 00
		//	constexpr std::uintptr_t kJSOffset = 0x61;

		//	REL::Relocation<std::uintptr_t> JSLoc{ REL::ID(36564), kJSOffset };
		//	SKSE::AllocTrampoline(8);
		//	SKSE::GetTrampoline().write_branch<6>(
		//		JSLoc.address(),
		//		JSLoc.address() + 0x06
		//	);
		//}
	}

	void Install() {
		stl::write_thunk_call<Hook_RegisterClassA, 6>(REL::VariantID(75591, 77226, 0).address() + REL::VariantOffset(0x8E, 0x15C, 0).offset());
		stl::write_thunk_call<Hook_MainUpdate, 5>(REL::VariantID(35565, 36564, 0).address() + REL::VariantOffset(0x748, 0xC26, 0).offset());
		stl::write_thunk_call<Hook_InputDispatchEvent, 5>(REL::VariantID(67315, 68617, 0).address() + REL::VariantOffset(0x7B, 0x7B, 0).offset());
		RemoveCheck();
	}

}

SKSEPluginLoad(const LoadInterface * a_skse) {
	Init(a_skse);
	Install();

	if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
		switch (message->type) {
			case MessagingInterface::kPostLoadGame: {
				ResetInput();
				return;
			}
			default: {}
		}
	})) {
		SKSE::stl::report_and_fail("Could not register Messaging Interface");
	}
	


	return true;
}

SKSEPluginInfo(
	.Version = REL::Version{ 1, 0, 0, 0 },
	.Name = "AltTabFix",
	.Author = "BingusEx",
	.StructCompatibility = SKSE::StructCompatibility::Independent,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
);


