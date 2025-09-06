#pragma once

namespace Util::Win32 {

	static inline void ReportAndExit(const std::string_view& a_message) {
		MessageBoxA(
			nullptr,
			a_message.data(),
			PluginDeclaration::GetSingleton()->GetName().data(),
			MB_OK | MB_ICONERROR | MB_TOPMOST
		);
		TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
	}


	static inline void ReportInfo(const std::string_view& a_message) {
		MessageBoxA(
			nullptr,
			a_message.data(),
			PluginDeclaration::GetSingleton()->GetName().data(),
			MB_OK | MB_ICONINFORMATION | MB_TOPMOST
		);
	}
}