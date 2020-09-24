module PacketAnalyzer::NULL;

export {
	## Identifier mappings
	const dispatch_map: PacketAnalyzer::DispatchMap = {} &redef;
}

const DLT_NULL : count = 0;

redef PacketAnalyzer::ROOT::dispatch_map += {
	[DLT_NULL] = PacketAnalyzer::DispatchEntry($analyzer=PacketAnalyzer::ANALYZER_NULL)
};

redef dispatch_map += {
	[2] = PacketAnalyzer::DispatchEntry($analyzer=PacketAnalyzer::ANALYZER_IP),

	## From the Wireshark Wiki: AF_INET6ANALYZER, unfortunately, has different values in
	## {NetBSD,OpenBSD,BSD/OS}, {FreeBSD,DragonFlyBSD}, and {Darwin/Mac OS X}, so an IPv6
	## packet might have a link-layer header with 24, 28, or 30 as the AF_ value. As we
	## may be reading traces captured on platforms other than what we're running on, we
	## accept them all here.
	[24] = PacketAnalyzer::DispatchEntry($analyzer=PacketAnalyzer::ANALYZER_IP),
	[28] = PacketAnalyzer::DispatchEntry($analyzer=PacketAnalyzer::ANALYZER_IP),
	[30] = PacketAnalyzer::DispatchEntry($analyzer=PacketAnalyzer::ANALYZER_IP)
};
