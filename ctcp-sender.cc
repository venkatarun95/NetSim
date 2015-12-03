#include "ctcp-sender.hh"
#include "delay.hh"
#include "fifo-queue.hh"
#include "markoviancc.hh"
#include "multidelta-queue.hh"
#include "multi-queue.hh"
#include "pkt-logger.hh"
#include "rand-gen.hh"

#include <iostream>

using namespace std;

template< class CCC, class NextHop >
void CTCPSender<CCC, NextHop>::tick(TickNum tick_num) {
	assert(next_tick_num == tick_num);
	congctrl.set_timestamp(tick_num);

	if (cur_state == false)
		tick_num += 1e-4; 	// this is necessary because traffic generator may
							// not detect the need to switch on because
							// floating point comparison may fail. We must
							// switch on nevertheless.
	bool new_state = \
	  traffic_generator.switch_state(num_pkts_transmitted, tick_num, sender_id);
	if (new_state != cur_state) {
		cur_state = new_state;
		if (cur_state == false) {
			next_tick_num = tick_num + traffic_generator.num_ticks_to_switch_off(sender_id);
			cout << "Off till " << next_tick_num << endl;
		}
		else {
			num_pkts_transmitted = 0;
			num_pkts_acked = 0;
			last_sent_tick = tick_num;
			++ flow_id;
			congctrl.init();
			next_tick_num = tick_num + congctrl.get_intersend_time();
		}
		return;
	}
	assert (cur_state);
	if ((num_pkts_transmitted - num_pkts_acked) < congctrl.get_the_window() \
		&& tick_num - last_sent_tick >= congctrl.get_intersend_time()) {
		Packet pkt = {num_pkts_transmitted, sender_id, flow_id, tick_num,
			congctrl.get_delta_class()};
		// TODO(venkat): This is a hacky method that assumes that congctrl
		// is MarkovianCC. Fix this to a more modular method.
		next_hop.push_pkt(pkt, tick_num);
		++ num_pkts_transmitted;
		last_sent_tick = tick_num;

		congctrl.onPktSent(pkt.seq_num);
		cerr << "0:0:" << tick_num << " " << 1.0/congctrl.get_intersend_time() << " " << sender_id << " >" << endl;
	}
	if (congctrl.get_intersend_time() <= 0) {
		assert (false);
		// cerr << "Intersend time: " << congctrl.get_intersend_time() << " " << sender_id << endl;
		next_tick_num = tick_num + 1e-3;
	}
	else {
		next_tick_num = tick_num + congctrl.get_intersend_time() + \
			rand_gen.uniform(congctrl.get_intersend_time()*(-0.05), \
				0.05*congctrl.get_intersend_time());
	}

	assert (next_tick_num != tick_num);
}

template< class CCC, class NextHop >
void CTCPSender<CCC, NextHop>::push_pkt(Packet& pkt, TickNum tick_num) {
	if (pkt.flow_id < flow_id) {
		// cerr << "Leftover packet from previous flow detected." << endl;
		return;
	}
	assert(pkt.seq_num <= num_pkts_transmitted);
	assert(pkt.sender_id == sender_id);
	assert(pkt.send_time <= tick_num);

	congctrl.set_timestamp(tick_num);
	++ num_pkts_acked;

	congctrl.onACK(pkt.seq_num+1, tick_num);
}

template class CTCPSender< MarkovianCC, MultiQueue< PktLogger > >;
template class CTCPSender< MarkovianCC, FifoQueue< Delay < PktLogger > > >;
template class CTCPSender< MarkovianCC, MultiDeltaQueue< Delay < PktLogger > > >;
