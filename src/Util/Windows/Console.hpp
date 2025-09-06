#pragma once
#include "MessageBox.hpp"

namespace Util::Win32 {

	// This function allocates a new console and redirects the standard streams
	static inline void AllocateConsole() {

		// Allocate a new console for the calling process.
		if (!AllocConsole()) {
			std::cerr << "Failed to allocate console" << '\n';
			ReportInfo("Could not allocate a console window...");
			return;
		}

		// Redirect unbuffered STDOUT to the console.
		FILE* fpOut = nullptr;
		if (freopen_s(&fpOut, "CONOUT$", "w", stdout) != 0) {
			std::cerr << "Failed to redirect stdout" << '\n';
			ReportInfo("stdout could not be redirected.");
		}

		// Redirect unbuffered STDERR to the console.
		FILE* fpErr = nullptr;
		if (freopen_s(&fpErr, "CONOUT$", "w", stderr) != 0) {
			std::cerr << "Failed to redirect stderr" << '\n';
		}

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		// Enable ANSI escape sequences for colored output (requires Windows 10+).
		DWORD mode = 0;
		if (GetConsoleMode(hConsole, &mode)) {
			mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			SetConsoleMode(hConsole, mode);
		}

		// Synchronize the C++ standard streams with the C standard streams.
		std::ios::sync_with_stdio();

	}
}
