// See the file  in the main distribution directory for copyright.

#include "RADIUS.h"
#include "plugin/Plugin.h"
#include "analyzer/Component.h"

namespace zeek::plugin::detail::Zeek_RADIUS {

class Plugin : public zeek::plugin::Plugin {
public:
	zeek::plugin::Configuration Configure() override
		{
		AddComponent(new zeek::analyzer::Component("RADIUS", zeek::analyzer::radius::RADIUS_Analyzer::Instantiate));

		zeek::plugin::Configuration config;
		config.name = "Zeek::RADIUS";
		config.description = "RADIUS analyzer";
		return config;
		}
} plugin;

} // namespace zeek::plugin::detail::Zeek_RADIUS
