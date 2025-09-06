#include "Util/Text/Text.hpp"

namespace Util::Text {

	bool StartsWith(std::string_view a_baseStr, std::string_view a_preffix) {
		return a_baseStr.compare(0, a_preffix.size(), a_preffix);
	}

	bool Regex_Matches(std::string_view a_baseStr, std::string_view a_reg) {
		std::regex reg(std::string(a_reg).c_str());
		return std::regex_match(std::string(a_baseStr), reg);
	}

	std::string ToLower(std::string a_str) {
		std::ranges::transform(a_str, a_str.begin(),[](unsigned char c){
			return std::tolower(c);
		});
		return a_str;
	}

	std::string ToUpper(std::string a_str) {
		std::ranges::transform(a_str, a_str.begin(),[](unsigned char c){
			return std::toupper(c);
		});
		return a_str;
	}

	void ReplaceFirst(std::string& s, std::string const& toReplace, std::string const& replaceWith) {
		std::size_t pos = s.find(toReplace);
		if (pos == std::string::npos) {
			return;
		}
		s.replace(pos, toReplace.length(), replaceWith);
	}

	std::string RemoveWhitespace(std::string a_str) {
		a_str.erase(std::ranges::remove(a_str,' ').begin(), a_str.end());
		return a_str;
	}

	std::string Trim(const std::string& s) {
		auto start = s.begin();
		while (start != s.end() && std::isspace(static_cast<unsigned char>(*start))) {
			++start;
		}

		auto end = s.end();
		do {
			--end;
		}
		while (end != start && std::isspace(static_cast<unsigned char>(*end)));

		return std::string(start, end + 1);
	}

	void TrimL(std::string& s) {
		s.erase(s.begin(), std::ranges::find_if(s,[](unsigned char ch) {
			return !std::isspace(ch);
		}));
	}

	void TrimR(std::string& s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}


}
