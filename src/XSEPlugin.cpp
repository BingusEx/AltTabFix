
SKSEPluginLoad(const LoadInterface * a_skse) {
	Init(a_skse);
	Install();
	return(true);
}

SKSEPluginInfo(
	.Version = REL::Version{ 1, 0, 0, 0 },
	.Name = "TemplatePlugin",
	.Author = "BingusEx",
	.StructCompatibility = SKSE::StructCompatibility::Independent,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
);


