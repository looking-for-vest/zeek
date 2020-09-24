// See the file  in the main distribution directory for copyright.

#include <pcap.h>	// for the DLT_EN10MB constant definition

#include "VXLAN.h"
#include "TunnelEncapsulation.h"
#include "Conn.h"
#include "IP.h"
#include "RunState.h"
#include "Sessions.h"
#include "Reporter.h"
#include "packet_analysis/Manager.h"
#include "packet_analysis/protocol/iptunnel/IPTunnel.h"

#include "events.bif.h"

extern "C" {
#include <pcap.h>
}

namespace zeek::analyzer::vxlan {

void VXLAN_Analyzer::Done()
	{
	Analyzer::Done();
	Event(udp_session_done);
	}

void VXLAN_Analyzer::DeliverPacket(int len, const u_char* data, bool orig,
                                   uint64_t seq, const IP_Hdr* ip, int caplen)
	{
	Analyzer::DeliverPacket(len, data, orig, seq, ip, caplen);

	// Outer Ethernet, IP, and UDP layers already skipped.
	// Also, generic UDP analyzer already checked/guarantees caplen >= len.

	constexpr auto vxlan_len = 8;

	if ( len < vxlan_len )
		{
		ProtocolViolation("VXLAN header truncation", (const char*) data, len);
		return;
		}

	if ( (data[0] & 0x08) == 0 )
		{
		ProtocolViolation("VXLAN 'I' flag not set", (const char*) data, len);
		return;
		}

	EncapsulationStack* outer = Conn()->GetEncapsulation();

	if ( outer && outer->Depth() >= BifConst::Tunnel::max_depth )
		{
		reporter->Weird(Conn(), "tunnel_depth");
		return;
		}

	bool delete_outer = false;
	if ( ! outer )
		{
		outer = new EncapsulationStack();
		delete_outer = true;
		}

	EncapsulatingConn inner(Conn(), BifEnum::Tunnel::VXLAN);
	outer->Add(inner);

	int vni = (data[4] << 16) + (data[5] << 8) + (data[6] << 0);

	// Skip over the VXLAN header and create a new packet.
	data += vxlan_len;
	caplen -= vxlan_len;
	len -= vxlan_len;

	pkt_timeval ts;
	ts.tv_sec = (time_t) run_state::current_timestamp;
	ts.tv_usec = (suseconds_t) ((run_state::current_timestamp - (double)ts.tv_sec) * 1000000);
	Packet pkt(DLT_EN10MB, &ts, caplen, len, data);
	pkt.key_store["encap"] = outer;
	packet_mgr->ProcessPacket(&pkt);

	if ( ! pkt.l2_valid )
		{
		if ( delete_outer )
			delete outer;

		ProtocolViolation("VXLAN invalid inner ethernet frame",
		                  (const char*) data, len);
		return;
		}

	data += pkt.hdr_size;
	len -= pkt.hdr_size;
	caplen -= pkt.hdr_size;

	IP_Hdr* inner_hdr = nullptr;
	int res = 0;

	switch ( pkt.l3_proto ) {
		case L3_IPV4:
			res = sessions->ParseIPPacket(len, data, IPPROTO_IPV4, inner_hdr);
			break;
		case L3_IPV6:
			res = sessions->ParseIPPacket(len, data, IPPROTO_IPV6, inner_hdr);
			break;
		default:
			return;
	}

	if ( res < 0 )
		{
		delete inner_hdr;
		if ( delete_outer )
			delete outer;

		ProtocolViolation("Truncated VXLAN or invalid inner IP",
		                  (const char*) data, len);
		return;
		}

	ProtocolConfirmation();

	if ( vxlan_packet )
		Conn()->EnqueueEvent(vxlan_packet, nullptr, ConnVal(),
		                     inner_hdr->ToPktHdrVal(), val_mgr->Count(vni));

	if ( delete_outer )
		delete outer;
	}

} // namespace zeek::analyzer::vxlan
