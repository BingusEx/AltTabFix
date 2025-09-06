#include "Config/Config.hpp"
#include "Hooks/Hooks.hpp"
#include "Version.hpp"

#include "Util/Logger/Logger.hpp"

SKSEPluginLoad(const LoadInterface * a_skse) {

	Init(a_skse);
	logger::Initialize();
	Config::ConfigManager::Initialize();
	Hooks::Install();

	if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
		switch (message->type) {
			case MessagingInterface::kPostLoadGame: {
				if (Config::ConfigManager::AltTabFix.bEnable) {
					Hooks::AltTabFix::ResetInput();
				}
			}
			default: {}
		}
	})) {
		SKSE::stl::report_and_fail("Could not register Messaging Interface");
	}

	return true;
}

SKSEPluginInfo(
	.Version = Plugin::ModVersion,
	.Name = Plugin::ModName,
	.Author = "BingusEx",
	.StructCompatibility = SKSE::StructCompatibility::Independent,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
);


