#include <iostream>
#include <limits>

#include "network.hh"
#include "rand-gen.hh"

using namespace std;

// Network::Network(int num_senders, double time_unit __attribute((unused)))
// :	senders(),
// 	queue(10, 1.0, delay),
// 	delay(50, pkt_logger),
// 	pkt_logger(senders),
// 	traffic_generator(10000, 10000, "exponential")
// {
// 	vector< double > rand_nums(num_senders, 0);
// 	double sum_rand_nums = 0;
// 	for (auto & x : rand_nums) {
// 		x = rand_gen.uniform(0.0, 1.0);
// 		sum_rand_nums += x;
// 	}
// 	for (int i = 0;i < num_senders;i++) {
// 		senders.push_back(CTCPSender< MarkovianCC, MultiDeltaQueue< Delay< PktLogger > > >
// 			(MarkovianCC(0.1), queue, i, traffic_generator));
// 	}
// }

Network::Network(int num_senders __attribute((unused)), double time_unit)
:	senders(),
	queue(10, 1.0, delay),
	delay(10, pkt_logger),
	pkt_logger(senders),
	traffic_generator()//(1, 1, "")
{
	traffic_generator.push_back(TrafficGenerator(7*time_unit, 0*time_unit, "deterministic"));
	traffic_generator.push_back(TrafficGenerator(5*time_unit, 1*time_unit, "deterministic"));
	traffic_generator.push_back(TrafficGenerator(3*time_unit, 2*time_unit, "deterministic"));
	traffic_generator.push_back(TrafficGenerator(1*time_unit, 3*time_unit, "deterministic"));

	for (int i = 0;i < 4;i++) {
		double delta = 1;
		if (i < 2)
			delta = 2;
		senders.push_back(CTCPSender< MarkovianCC, MultiDeltaQueue< Delay< PktLogger > > >
			(MarkovianCC(delta), queue, i, traffic_generator[i]));
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
		next_tick = min(next_tick, delay.next_tick());
		assert(next_tick >= tick);

		if (next_tick == tick) {
			cout << "Simulation terminated before run duration." << endl;
			cout << "Ran till: " << tick << endl;
			for (auto & s : senders)
				cout << s.next_tick() << " ";
			cout << endl << queue.next_tick() << " " << delay.next_tick() << endl;
			break;
		}

		tick = next_tick;
		for (auto & s : senders)
			if (s.next_tick() == tick)
				s.tick(tick);
		if (queue.next_tick() == tick)
			queue.tick(tick);
		if (delay.next_tick() == tick)
			delay.tick(tick);
	}
	pkt_logger.print_logged_stats(min(tick, run_duration));
}
