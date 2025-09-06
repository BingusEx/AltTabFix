#pragma once

namespace Util::Text {

	template< typename ... Args >
	void PrintNotification(std::string_view rt_fmt_str, Args&&... args) {
		try {
			DebugNotification(std::vformat(rt_fmt_str, std::make_format_args(args ...)).c_str());
		} catch (const std::format_error &e) {
			logger::warn("Could not format notification, check valid format string: {}", e.what());
		}
	}

	template< typename ... Args >
	void PrintMessageBox(std::string_view rt_fmt_str, Args&&... args) {
		try {
			DebugMessageBox(std::vformat(rt_fmt_str, std::make_format_args(args ...)).c_str());
		} catch (const std::format_error &e) {
			logger::warn("Could not format notification, check valid format string: {}", e.what());
		}
	}

	template< typename ... Args >
	void PrintConsole(std::string_view rt_fmt_str, Args&&... args) {
		try {
			ConsoleLog::GetSingleton()->Print("%s", std::vformat(rt_fmt_str, std::make_format_args(args ...)).c_str());
		} catch (const std::format_error &e) {
			logger::warn("Could not format console log, check valid format string: {}", e.what());
		}
	}

	std::string ToLower(std::string a_str);
	std::string ToUpper(std::string a_str);

	bool StartsWith(std::string_view a_baseStr, std::string_view a_preffix);
	bool Regex_Matches(std::string_view a_baseStr, std::string_view a_reg);

	void ReplaceFirst(std::string& s, std::string const& toReplace, std::string const& replaceWith);
	std::string RemoveWhitespace(std::string a_str);

	std::string Trim(const std::string& s);
	void TrimL(std::string& s);
	void TrimR(std::string& s);

}
