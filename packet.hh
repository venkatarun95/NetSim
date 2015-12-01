#ifndef PACKET_HH
#define PACKET_HH

#include "utilities.hh"

class Packet {
public:
	unsigned int seq_num;
	SenderId sender_id;
	FlowId flow_id;
	TickNum send_time;
	// Each MarkovianCC sender has a delta belonging to one of several
	// classes. This identifies the class to which the current flow
	// belongs.
	int delta_class;
};

#endif
