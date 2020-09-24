// See the file "COPYING" in the main distribution directory for copyright.

#pragma once

#include "util.h" // for bro_uint_t
#include "IPAddr.h"
#include "Reassem.h"
#include "Timer.h"

#include <tuple>

#include <sys/types.h> // for u_char

ZEEK_FORWARD_DECLARE_NAMESPACED(NetSessions, zeek);
ZEEK_FORWARD_DECLARE_NAMESPACED(IP_Hdr, zeek);
ZEEK_FORWARD_DECLARE_NAMESPACED(FragReassembler, zeek::detail);
ZEEK_FORWARD_DECLARE_NAMESPACED(FragTimer, zeek::detail);

namespace zeek::detail {

using FragReassemblerKey = std::tuple<IPAddr, IPAddr, bro_uint_t>;

class FragReassembler : public Reassembler {
public:
	FragReassembler(NetSessions* s, const IP_Hdr* ip, const u_char* pkt,
	                const FragReassemblerKey& k, double t);
	~FragReassembler() override;

	void AddFragment(double t, const IP_Hdr* ip, const u_char* pkt);

	void Expire(double t);
	void DeleteTimer();
	void ClearTimer()	{ expire_timer = nullptr; }

	IP_Hdr* ReassembledPkt()	{ return reassembled_pkt; }
	const FragReassemblerKey& Key() const	{ return key; }

protected:
	void BlockInserted(DataBlockMap::const_iterator it) override;
	void Overlap(const u_char* b1, const u_char* b2, uint64_t n) override;
	void Weird(const char* name) const;

	u_char* proto_hdr;
	IP_Hdr* reassembled_pkt;
	NetSessions* s;
	uint64_t frag_size;	// size of fully reassembled fragment
	FragReassemblerKey key;
	uint16_t next_proto; // first IPv6 fragment header's next proto field
	uint16_t proto_hdr_len;

	FragTimer* expire_timer;
};

class FragTimer final : public Timer {
public:
	FragTimer(FragReassembler* arg_f, double arg_t)
		: Timer(arg_t, TIMER_FRAG)
			{ f = arg_f; }
	~FragTimer() override;

	void Dispatch(double t, bool is_expire) override;

	// Break the association between this timer and its creator.
	void ClearReassembler()	{ f = nullptr; }

protected:
	FragReassembler* f;
};

class FragmentManager {
public:

	FragmentManager() = default;
	~FragmentManager();

	FragReassembler* NextFragment(double t, const IP_Hdr* ip, const u_char* pkt);
	void Clear();
	void Remove(detail::FragReassembler* f);

	size_t Size() const	{ return fragments.size(); }
	size_t MaxFragments() const 	{ return max_fragments; }
	uint32_t MemoryAllocation() const;

private:

	using FragmentMap = std::map<detail::FragReassemblerKey, detail::FragReassembler*>;
	FragmentMap fragments;
	size_t max_fragments = 0;
};

extern FragmentManager* fragment_mgr;

class FragReassemblerTracker {
public:
	FragReassemblerTracker(FragReassembler* f)
		: frag_reassembler(f)
		{ }

	~FragReassemblerTracker()
		{ fragment_mgr->Remove(frag_reassembler); }

private:
	FragReassembler* frag_reassembler;
};

} // namespace zeek::detail
