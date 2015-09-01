#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

#include "multi-queue.hh"
#include "packet.hh"
#include "utilities.hh"

using namespace std;

#undef QUEUE_LOGGING

template< class T >
void MultiQueue< T >::push_pkt(Packet& pkt, TickNum tick_num) {
	// #ifdef QUEUE_LOGGING
	// cout << "Packet: " << pkt.flow_id << " " << pkt.seq_num << " " << pkt.send_time << endl;
	// #endif
	// update flow intersend time
	if (flow_throughputs.count(pkt.flow_id) == 0)
		flow_throughputs[pkt.flow_id] = make_pair(tick_num, 
			numeric_limits<TickNum>::max());
	else {
		if (flow_throughputs[pkt.flow_id].second == numeric_limits<TickNum>::max())
			flow_throughputs[pkt.flow_id].second = tick_num - \
				flow_throughputs[pkt.flow_id].first;
		else
			flow_throughputs[pkt.flow_id].second = \
				alpha_intersend_time * (tick_num - flow_throughputs[pkt.flow_id].first) \
				+ (1 - alpha_intersend_time) * flow_throughputs[pkt.flow_id].second;
	}
	flow_throughputs[pkt.flow_id].first = tick_num;

	// find ideal queue length
	unsigned int sum_queue_lengths = 0;
	for (const auto & q : queues) 
		sum_queue_lengths += q.size();
	double total_throughput = 0;
	for (const auto & f : flow_throughputs) {
		if (f.second.second != numeric_limits<TickNum>::max())
			total_throughput += 1.0 / f.second.second;
	}
	
	double throughput = 1.0/flow_throughputs[pkt.flow_id].second;

	double ideal_queue_length;
	if (sum_queue_lengths == 1) // divide by 0 case
		ideal_queue_length = 0;
	else{
		// ideal_queue_length = throughput * sum_queue_lengths * 
		// sum_queue_lengths * intersend_time / (sum_queue_lengths - 1);
		if (total_throughput == 0)
			ideal_queue_length = 0;
		else
			ideal_queue_length = throughput * sum_queue_lengths / total_throughput;
	}

	// choose queue
	unsigned int queue_id = numeric_limits<unsigned int>::max();
	double min_queue_dist = numeric_limits<double>::max();

	for (unsigned i = 0;i < queues.size();i++) {
		if (fabs(ideal_queue_length - queues[i].size()) < min_queue_dist && \
			(i == 0 || ideal_queue_length > queues[i].size())) {
			min_queue_dist = fabs(ideal_queue_length - queues[i].size());
			queue_id = i;
		}
	}

	// if (queues[queue_id].size() > 1000){
	// 	#ifdef QUEUE_LOGGING
	// 	cout << "Dropped packet from  " << queue_id << endl;
	// 	#endif
	// 	return;
	// }
	// static double avg_error = 0;
	// static unsigned num_err_readings = 0;
	// avg_error += abs(queues[queue_id].size() - ideal_queue_length);
	// num_err_readings += 1;
	// if (num_err_readings % 100000 == 0)
	// 	cout << avg_error / num_err_readings << endl;

	#ifdef QUEUE_LOGGING
	if (queues[queue_id].size() < ideal_queue_length) {
		cout << "Pushed to " << queue_id \
		<< ". Queue size (ideal/actual): " << queues[queue_id].size() << " " << ideal_queue_length \
		<< ". Throughput|Flow: " << throughput << " | " << pkt.flow_id << endl;
	}
	#endif

	queues[queue_id].push(pkt);
}

template< class T >
void MultiQueue< T >::tick(TickNum tick_num) {
	assert(tick_num > last_sent_tick);
	if (last_sent_tick + intersend_time > tick_num)
		return;
	unsigned i = (last_served_queue+1) % queues.size();
	do {
		if (!queues[i].empty()) {
			#ifdef QUEUE_LOGGING
			cout << "Popped " << i << " at " << tick_num << " " << endl;
			#endif
			last_served_queue = i;
			next_hop.push_pkt(queues[i].front(), tick_num);
			queues[i].pop();
			break;
		}
		i = (i + 1) % queues.size();
	} while(i != (last_served_queue+1) % queues.size());
	last_sent_tick = tick_num; // the opportunity was wasted
}

template void MultiQueue< PktLogger >::push_pkt(Packet& pkt, TickNum tick_num);
template void MultiQueue< PktLogger >::tick(TickNum tick_num);
