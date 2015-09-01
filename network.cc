#include <iostream>
#include <limits>

#include "network.hh"
#include "rand-gen.hh"

using namespace std;

Network::Network(int num_senders)
:	senders(),
	queue(256, 1.0, pkt_logger),
	pkt_logger()
{
	vector< double > rand_nums(num_senders, 0);
	double sum_rand_nums = 0;
	for (auto & x : rand_nums) {
		x = rand_gen.uniform(0.0, 1.0);
		sum_rand_nums += x;
	}
	for (int i = 0;i < num_senders;i++) {
		senders.push_back(Sender<MultiQueue<PktLogger>>(sum_rand_nums/(rand_nums[i]*0.97), queue, i));
	}
}

void Network::run(TickNum run_duration) {
	TickNum tick = 0;
	while (tick < run_duration) {
		TickNum next_tick = numeric_limits<TickNum>::max();
		for (auto & s : senders) {
			next_tick = min(next_tick, s.next_tick());
		}

		next_tick = min(next_tick, queue.next_tick());
		assert(next_tick >= tick);

		if (next_tick == tick) {
			cout << "Simulation terminated before run duration." << endl;
			cout << "Ran till: " << tick << endl;
			break;
		}

		tick = next_tick;
		for (auto & s : senders)
			s.tick(tick);
		queue.tick(tick);
	}
	pkt_logger.print_logged_stats(min(tick, run_duration));
}