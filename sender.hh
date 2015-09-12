#ifndef SENDER_HH
#define SENDER_HH

#include <cassert>
#include <iostream>

#include "rand-gen.hh"
#include "utilities.hh"

template< class T>
class ConstantRateSender {
	unsigned int sender_id;
	T& next_hop;
	TickNum avg_intersend_time;
	TickNum last_sent_tick;
	TickNum next_tick_to_transmit;
	unsigned int num_pkts_transmitted;

public:
	ConstantRateSender(TickNum s_avg_intersend_time, T& s_next_hop, unsigned int s_sender_id)
	:	sender_id(s_sender_id),
		next_hop(s_next_hop),
		avg_intersend_time(s_avg_intersend_time),
		last_sent_tick(),
		next_tick_to_transmit(avg_intersend_time),
		num_pkts_transmitted()
	{}
	
	TickNum next_tick() const {return next_tick_to_transmit;}

	void tick(TickNum tick_num) {
		// cout << sender_id << " " << tick_num << endl;
		assert(tick_num > last_sent_tick);
		assert(tick_num <= next_tick_to_transmit);
		if (tick_num < next_tick_to_transmit)
			return;

		Packet pkt = {num_pkts_transmitted, sender_id, tick_num};
		next_hop.push_pkt(pkt, tick_num);
		last_sent_tick = tick_num;
		++ num_pkts_transmitted;

		next_tick_to_transmit = tick_num + rand_gen.exponential(1 / avg_intersend_time);
		// std::cout << next_tick_to_transmit - tick_num << " " << avg_intersend_time << std::endl;
	}
};

#endif