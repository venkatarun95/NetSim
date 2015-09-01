#ifndef NETWORK_HH
#define NETWORK_HH

#include <vector>

#include "pkt-logger.hh"
#include "multi-queue.hh"
#include "sender.hh"
#include "utilities.hh"

class Network {
	std::vector< Sender<MultiQueue< PktLogger > > > senders;
	MultiQueue< PktLogger > queue;
	PktLogger pkt_logger;

public:
	Network(int num_senders);
	void run(TickNum run_duration);
};

#endif