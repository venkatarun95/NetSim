#ifndef FIFO_QUEUE
#define FIFO_QUEUE

#include "packet.hh"
#include "utilities.hh"

#include <queue>

template< class NextHop >
class FifoQueue {
	std::queue< Packet > queue;
	TickNum intersend_time;
	NextHop& next_hop;

	TickNum last_sent_tick;

public:
	FifoQueue(const TickNum s_intersend_time, NextHop& s_next_hop)
	:	queue(),
		intersend_time(s_intersend_time),
		next_hop(s_next_hop),
		last_sent_tick()
	{}

	void push_pkt(Packet& pkt, TickNum tick_num __attribute((unused))) {
		queue.push(pkt);
	}
	void tick(TickNum tick_num) {
		assert(tick_num == last_sent_tick + intersend_time);
		last_sent_tick = tick_num;
		if (!queue.empty()) {
			next_hop.push_pkt(queue.front(), tick_num);
			queue.pop();
		}
	}
	TickNum next_tick() const {return last_sent_tick + intersend_time;}
};
#endif