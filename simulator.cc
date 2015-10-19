#include <iostream>

#include "multi-queue.hh"
#include "network.hh"

int main() {
	// MultiQueue queue(10, 1);
	// Packet pkt;
	// pkt.seq_num = 0;
	// pkt.flow_id = 1;
	// queue.push_pkt(pkt, 0);
	// queue.push_pkt(pkt, 0.5);
	// queue.tick(0.5);
	// queue.tick(1.5);
	// queue.push_pkt(pkt, 1.3);
	// queue.push_pkt(pkt, 1.2);
	// queue.push_pkt(pkt, 1.4);
	// queue.tick(3.5);
	// queue.tick(5.5);
	// queue.tick(6.5);
	// queue.tick(8.5);

	Network net(4, 5000);
	// Network net(4);
	net.run(350000);

	return 0;
}