#ifndef PACKET_HH
#define PACKET_HH

#include "utilities.hh"

class Packet {
public:
	unsigned int seq_num;
	SenderId sender_id;
	FlowId flow_id;
	TickNum send_time;
};

#endif