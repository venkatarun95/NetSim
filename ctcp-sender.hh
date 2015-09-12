#ifndef CTCP_SENDER_HH
#define CTCP_SENDER_HH

#include "packet.hh"
#include "traffic-generator.hh"
#include "utilities.hh"

template< class CCC, class NextHop >
class CTCPSender {
	SenderId sender_id;
	NextHop& next_hop;
	
	TickNum last_sent_tick;
	TickNum next_tick_num;
	unsigned int num_pkts_transmitted;
	unsigned int num_pkts_acked;

	// On - true, off - false
	TrafficGenerator& traffic_generator;
	bool cur_state;
	FlowId flow_id;

	CCC congctrl;

public:
	CTCPSender(
		CCC s_congctrl, \
		NextHop& s_next_hop, \
		SenderId s_sender_id, \
		TrafficGenerator& s_traffic_generator)
	:	sender_id(s_sender_id),
		next_hop(s_next_hop),
		last_sent_tick(0),
		next_tick_num(1),
		num_pkts_transmitted(0),
		num_pkts_acked(0),
		traffic_generator(s_traffic_generator),
		cur_state(true),
		flow_id(0),
		congctrl(s_congctrl)
	{
		congctrl.init();
	}

	void tick(TickNum tick_num);
	void push_pkt(Packet& pkt, TickNum tick_num);

	TickNum next_tick() const {return next_tick_num;}
};

#endif