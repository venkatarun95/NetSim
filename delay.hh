#ifndef DELAY_HH
#define DELAY_HH

#include "packet.hh"
#include "utilities.hh"

#include <iostream>
#include <limits>
#include <queue>
#include <tuple>

template <class NextHop>
class Delay {
	NextHop& next_hop;
	std::queue< std::pair< Packet, TickNum > > pkts;

	TickNum delay_amt;

public:
	Delay(TickNum s_delay_amt, NextHop& s_next_hop)
	:	next_hop(s_next_hop),
		pkts(),
		delay_amt(s_delay_amt)
	{}

	void push_pkt(Packet& pkt, TickNum tick_num) {
		if (!pkts.empty() && tick_num < pkts.front().second)
			std::cout << "lkadfy " << tick_num << " " << pkts.front().second << std::endl << std::flush;
		assert( pkts.empty() || tick_num >= pkts.front().second);
		pkts.push(std::make_pair(pkt, tick_num));
	}

	void tick(TickNum tick_num) {
		assert(pkts.front().second + delay_amt == tick_num);
		while (!pkts.empty() && pkts.front().second + delay_amt <= tick_num) {
			assert(pkts.front().second + delay_amt == tick_num);
			next_hop.push_pkt(pkts.front().first, tick_num);
			pkts.pop();
		}
	}

	TickNum next_tick() {
		if (pkts.empty())
			return std::numeric_limits<TickNum>::max();
		return pkts.front().second + delay_amt;
	}
};

#endif
