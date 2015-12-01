#ifndef PKT_LOGGER_HH
#define PKT_LOGGER_HH

#include "ctcp-sender.hh"
#include "delay.hh"
#include "fifo-queue.hh"
#include "markoviancc.hh"
#include "multidelta-queue.hh"
#include "packet.hh"
#include "utilities.hh"

#include <map>
#include <tuple>
#include <vector>

class PktLogger {
	typedef CTCPSender< MarkovianCC, MultiDeltaQueue< Delay< PktLogger > > > SenderType;

	struct FlowStats {
		TickNum flow_start_time; // for calculating average throughput
		TickNum flow_end_time;
		TickNum sum_pkt_delays; // for calculating average delay
		unsigned int num_pkts_logged;
	};

	std::map< std::pair< SenderId, FlowId >, FlowStats > flow_stats;

	std::vector< SenderType >& senders;
public:
	PktLogger(std::vector< SenderType >& s_senders)
	:	flow_stats(),
		senders(s_senders)
	{}
	void push_pkt(Packet& pkt, TickNum tick_num);
	void print_logged_stats(TickNum tick_num);
};

#endif
