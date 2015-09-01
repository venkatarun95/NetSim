#include <iostream>

#include "pkt-logger.hh"

using namespace std;

void PktLogger::push_pkt(Packet& pkt, TickNum tick_num) {
	if (flow_stats.count(pkt.flow_id) == 0) {
		flow_stats[pkt.flow_id] = {
			tick_num, // flow_start_time
			0.0, // sum_pkt_delays
			0 // num_pkts_logged
		};
	}
	++ flow_stats[pkt.flow_id].num_pkts_logged;
	flow_stats[pkt.flow_id].sum_pkt_delays += tick_num - pkt.send_time;
}

void PktLogger::print_logged_stats(TickNum tick_num) {
	cout << "FlowId Avg., Delay Avg., Tpt" << endl;
	for (auto & flow : flow_stats) {
		cout << flow.first << ", " \
			 << flow.second.sum_pkt_delays / flow.second.num_pkts_logged  << ", "\
			 << flow.second.num_pkts_logged / (tick_num - flow.second.flow_start_time) << endl;
	}
}