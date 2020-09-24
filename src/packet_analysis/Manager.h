// See the file "COPYING" in the main distribution directory for copyright.

#pragma once

#include "Tag.h"
#include "Component.h"
#include "plugin/ComponentManager.h"
#include "iosource/Packet.h"
#include "Dispatcher.h"

ZEEK_FORWARD_DECLARE_NAMESPACED(PacketProfiler, zeek::detail);

namespace zeek {
namespace packet_analysis {

class Analyzer;
using AnalyzerPtr = std::shared_ptr<Analyzer>;

class Manager : public plugin::ComponentManager<Tag, Component> {
public:
	/**
	 * Constructor.
	 */
	Manager();

	/**
	 * Destructor.
	 */
	~Manager();

	/**
	 * Second-stage initialization of the manager. This is called late
	 * during Zeek's initialization after any scripts are processed.
	 */
	void InitPostScript();

	/**
	 * Finished the manager's operations.
	 */
	void Done();

	/**
	 * Dumps out the state of all registered analyzers to the \c analyzer
	 * debug stream. Should be called only after any \c zeek_init events
	 * have executed to ensure that any of their changes are applied.
	 */
	void DumpDebug(); // Called after zeek_init() events.

	/**
	 * Looks up an analyzer instance.
	 *
	 * @param val The analyzer's tag value.
	 *
	 * @return The analyzer instance or nullptr if no instance is found.
	 */
	AnalyzerPtr GetAnalyzer(EnumVal *val);

	/**
	 * Looks up an analyzer instance.
	 *
	 * @param name The name of the analyzer.
	 *
	 * @return The analyzer instance or nullptr if no instance is found.
	 */
	AnalyzerPtr GetAnalyzer(const std::string& name);

	/**
	 * Processes a packet by applying the configured packet analyzers.
	 *
	 * @param packet The packet to process.
	 */
	void ProcessPacket(Packet* packet);

	/**
	 * Process the inner packet of an encapsulation. This can be used by tunnel
	 * analyzers to process a inner packet from the "beginning" directly through
	 * the root analyzer. This short-circuits some of the additional processing
	 * that happens in ProcessPacket().
	 *
	 * @param packet The packet to process.
	 */
	bool ProcessInnerPacket(Packet* packet);

	uint64_t PacketsProcessed() const	{ return num_packets_processed; }

	/**
	 * Records the given packet if a dumper is active.
	 *
	 * @param pkt The packet to record.
	 * @param len The number of bytes to record. If set to zero, the whole
	 * packet is recorded.
	 */
	void DumpPacket(const Packet *pkt, int len=0);

private:
	/**
	 * Instantiates a new analyzer instance.
	 *
	 * @param tag The analyzer's tag.
	 *
	 * @return The new analyzer instance. Returns null if tag is invalid, the
	 * requested analyzer is disabled, or the analyzer can't be instantiated.
	 */
	AnalyzerPtr InstantiateAnalyzer(const Tag& tag);

	/**
	 * Instantiates a new analyzer.
	 *
	 * @param name The name of the analyzer.
	 *
	 * @return The new analyzer instance. Returns null if the name is not known
	 * or if the requested analyzer that is disabled.
	 */
	AnalyzerPtr InstantiateAnalyzer(const std::string& name);

	std::map<std::string, AnalyzerPtr> analyzers;
	AnalyzerPtr root_analyzer = nullptr;

	uint64_t num_packets_processed = 0;
	detail::PacketProfiler* pkt_profiler = nullptr;
};

} // namespace packet_analysis

extern zeek::packet_analysis::Manager* packet_mgr;

} // namespace zeek
