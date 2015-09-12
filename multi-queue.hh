#ifndef MULTI_QUEUE_HH
#define MULTI_QUEUE_HH

#include <map>
#include <tuple>
#include <queue>
#include <vector>

#include "packet.hh"
#include "pkt-logger.hh"
#include "utilities.hh"

template< class T >
class MultiQueue {
	const double alpha_intersend_time = 1.0/128.0;

	T& next_hop;

	std::vector< std::queue< Packet > > queues;
	unsigned int last_served_queue;
	TickNum intersend_time;
	TickNum last_sent_tick;

	// Format: flow_id: (last received tuple, average intersend time)
	std::map< FlowId, std::pair< TickNum, TickNum > > flow_throughputs;
public:
	MultiQueue(const int num_queues, const TickNum s_intersend_time, T& s_next_hop)
	:	next_hop(s_next_hop),
		queues(),
		last_served_queue(),
		intersend_time(s_intersend_time),
		last_sent_tick(),
		flow_throughputs()
	{
		for (int i = 0;i < num_queues;i++){
			queues.emplace_back();
			// queues[i].push({0,0,0});
		}
	}
	void push_pkt(Packet& pkt, TickNum tick_num);
	void tick(TickNum tick_num);
	TickNum next_tick() const {return last_sent_tick + intersend_time;}
};

#endif