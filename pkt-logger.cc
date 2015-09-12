#include <iostream>

#include "pkt-logger.hh"

using namespace std;

void PktLogger::push_pkt(Packet& pkt, TickNum tick_num) {
	auto pkt_id = make_pair( pkt.sender_id, pkt.flow_id);
	
	// if new flow, create record
	if (flow_stats.count(pkt_id) == 0) {
		flow_stats[pkt_id] = {
			tick_num, // flow_start_time
			tick_num, // flow_end_time
			0.0, // sum_pkt_delays
			0 // num_pkts_logged
		};
	}

	// update flow statistics
	++ flow_stats[pkt_id].num_pkts_logged;
	flow_stats[pkt_id].sum_pkt_delays += tick_num - pkt.send_time;
	flow_stats[pkt_id].flow_end_time = tick_num;

	// send ack
	senders[pkt.sender_id].push_pkt(pkt, tick_num);
}

void PktLogger::print_logged_stats(TickNum tick_num __attribute((unused))) {
	double tot_tpt = 0, avg_del = 0;
	cout << "SenderId, FlowId, Avg.Delay, Avg.Tpt, FlowTime" << endl;
	for (auto & flow : flow_stats) {
		cout << flow.first.first << ", " << flow.first.second << ", " \
			 << flow.second.sum_pkt_delays / flow.second.num_pkts_logged  << ", "\
			 << flow.second.num_pkts_logged / (flow.second.flow_end_time - flow.second.flow_start_time) << ", "\
			 << flow.second.flow_end_time - flow.second.flow_start_time << endl;
		tot_tpt += flow.second.num_pkts_logged / (flow.second.flow_end_time - flow.second.flow_start_time);
		avg_del += flow.second.sum_pkt_delays / flow.second.num_pkts_logged;
	}
	cout << "'Total' Throughput: " << tot_tpt << " Average Delay: " << avg_del / flow_stats.size() << endl;
}

// template class PktLogger< CTCPSender< MarkovianCC, FifoQueue< PktLogger > > >;