#ifndef NETWORK_HH
#define NETWORK_HH

#include <vector>

#include "ctcp-sender.hh"
#include "delay.hh"
#include "fifo-queue.hh"
#include "markoviancc.hh"
#include "multi-queue.hh"
#include "pkt-logger.hh"
#include "utilities.hh"

class Network {
	std::vector< CTCPSender< MarkovianCC, FifoQueue< Delay< PktLogger > > > > senders;
	FifoQueue< Delay< PktLogger > > queue;
	Delay< PktLogger > delay;
	PktLogger pkt_logger;
	
	std::vector< TrafficGenerator > traffic_generator;

public:
	Network(int num_senders, double time_unit);
	void run(TickNum run_duration);
};

#endif