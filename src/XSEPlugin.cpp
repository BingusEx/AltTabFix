#include "Hook.hpp"
#include "Fixes.hpp"

extern "C" void DLLEXPORT APIENTRY Initialize() {
	//std::cout << "AltTabFix PreHook Installed\n";
}

SKSEPluginLoad(const LoadInterface * a_skse) {

	Init(a_skse);
	Hook::Install();

	if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
		switch (message->type) {
			case MessagingInterface::kPostLoadGame: {
				AltTabFix::ResetInput();
			}
			default: {}
		}
	})) {
		SKSE::stl::report_and_fail("Could not register Messaging Interface");
	}

	return true;
}

SKSEPluginInfo(
	.Version = REL::Version{ 1, 0, 2, 0 },
	.Name = "AltTabFix",
	.Author = "BingusEx",
	.StructCompatibility = SKSE::StructCompatibility::Independent,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
);


