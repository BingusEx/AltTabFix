#include "Logger.hpp"

#include "Util/Text/Text.hpp"
#include "Util/Windows/MessageBox.hpp"

namespace {

	//ANSI Escape Codes: Colors
	#define BLK "\033[30m"  //Black
	#define RED "\033[31m"  //Red
	#define GRN "\033[32m"  //Green
	#define YEL "\033[33m"  //Yellow
	#define BLU "\033[34m"  //Blue
	#define MAG "\033[35m"  //Magenta
	#define CYA "\033[36m"  //Cyan
	#define WHT "\033[37m"  //White

	//ANSI Escape Codes: Formating
	#define RST "\033[0m"   //Reset
	#define BLD "\033[1m"   //Bold
	#define UDL "\033[4m"   //Underline

	constexpr const char* PatternDefault = "[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v";
	constexpr const char* PatternConsole = WHT "[" RED "ATF" WHT "][%H:%M:%S.%e][%^%l%$][" YEL "%s:%#" WHT "]: %v" RST;

}

namespace SKSE::log {

	void Initialize() {

		auto path = log_directory();

		if (!path) {
			Util::Win32::ReportAndExit("Could not find a valid log directory. The game will now close");
		}

		*path /= PluginDeclaration::GetSingleton()->GetName();
		*path += L".log";

		std::shared_ptr <spdlog::logger> logger;

		if (HasConsole()) {
			logger = std::make_shared <spdlog::logger>("Global", std::make_shared <spdlog::sinks::stdout_color_sink_mt>());
			logger->set_pattern(PatternConsole);
			spdlog::set_default_logger(std::move(logger));
			SetLevel("Trace");
		}
		/*else if (IsDebuggerPresent()) {
			logger = std::make_shared <spdlog::logger>("Global", std::make_shared <spdlog::sinks::msvc_sink_mt>());
			logger->set_pattern(PatternDefault);
		}*/
		else {
			logger = std::make_shared <spdlog::logger>("Global", std::make_shared <spdlog::sinks::basic_file_sink_mt>(path->string(), true));
			logger->set_pattern(PatternDefault);
			spdlog::set_default_logger(std::move(logger));
			SetLevel("Info"); //Default level
		}
	}

	void SetLevel(spdlog::level::level_enum a_level) {
		spdlog::set_level(a_level);
		spdlog::flush_on(a_level);
	}

	void SetLevel(const char* a_level) {

		const auto to_level_enum = [](const char* levelStr) -> std::optional<spdlog::level::level_enum> {
			using enum spdlog::level::level_enum;

			std::string lower = Util::Text::ToLower(levelStr);
			if (lower == "off")                         return off;
			if (lower == "trace")                       return trace;
			if (lower == "debug")                       return debug;
			if (lower == "info")                        return info;
			if (lower == "warning" || lower == "warn")  return warn;
			if (lower == "error" || lower == "err")     return err;
			if (lower == "critical")                    return critical;

			return std::nullopt;
		};

		if (const auto level = to_level_enum(a_level)) {
			SetLevel(*level);
		}
	}
}