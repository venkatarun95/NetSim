#ifndef PKT_LOGGER_HH
#define PKT_LOGGER_HH

#include <unordered_map>

#include "packet.hh"
#include "utilities.hh"

class PktLogger {
	struct FlowStats {
		TickNum flow_start_time; // for calculating average throughput
		TickNum sum_pkt_delays; // for calculating average delay
		unsigned int num_pkts_logged;
	};

	std::unordered_map< FlowId, FlowStats > flow_stats;
public:
	PktLogger() : flow_stats() {}
	void push_pkt(Packet& pkt, TickNum tick_num);
	void print_logged_stats(TickNum tick_num);
};

#endif