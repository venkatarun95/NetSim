#include "markoviancc.hh"

#include "rand-gen.hh"

#include <boost/math/special_functions/fpclassify.hpp>
#include <cassert>
#include <cmath>
#include <limits>

using namespace std;

double MarkovianCC::current_timestamp( void ){
	#ifdef SIMULATION_MODE
	return cur_tick;

	#else
	using namespace std::chrono;
	high_resolution_clock::time_point cur_time_point = \
		high_resolution_clock::now();
	return duration_cast<duration<double>>(cur_time_point - start_time_point)\
		.count()*1000;
	#endif
}

void MarkovianCC::init() {
	if (num_pkts_acked != 0)
		cout << "%% Packets lost: " << (100.0*num_pkts_lost)/(num_pkts_acked+num_pkts_lost) << endl;
	unacknowledged_packets.clear();

	min_rtt = numeric_limits<double>::max();

	rtt_acked_ewma = 0.0;
	rtt_unacked_ewma = 0.0;
 	_intersend_time = initial_intersend_time;
	sending_rate = 1.0 / initial_intersend_time;
	sending_rate_velocity = 0.0;
	prev_ack_timestamp = 0.0;

	_the_window = numeric_limits<double>::max();
	_timeout = 1000;

	num_pkts_lost = num_pkts_acked = 0;

	start_time_point = std::chrono::high_resolution_clock::now();
}

void MarkovianCC::update_sending_rate(const PktInformation& pkt_info) {
	const double cur_time = current_timestamp();

	double rtt_ewma = max(rtt_unacked_ewma, rtt_acked_ewma);

	double spare_rate = 1.0 / (rtt_ewma - min_rtt);

	// to avoid bursts due to incorrect min_rtt estimate
	if (num_pkts_acked < 10) {
		// 	sending_rate = max(min(spare_rate / delta, sending_rate), 0.0);
		// _intersend_time = 1.0 / sending_rate;
	 	return;
	}

	double force = spring_k * (spare_rate / delta - pkt_info.sending_rate);
	force -= damping * pkt_info.sending_rate_velocity;
	const double dt = cur_time - prev_ack_timestamp;
	prev_ack_timestamp = cur_time;
	cout << sending_rate << " " << sending_rate_velocity << " " << dt << " " << spare_rate << " " << pkt_info.sending_rate << endl;
	sending_rate += pkt_info.sending_rate_velocity * dt;
	sending_rate_velocity = pkt_info.sending_rate_velocity + force / mass * dt;
	sending_rate_velocity = min(sending_rate / dt, sending_rate_velocity);

	assert(dt > 0);

	if (sending_rate < 1.0 / initial_intersend_time)
		sending_rate = 1.0 / initial_intersend_time;

	_intersend_time = 1.0 / sending_rate;
	assert(_intersend_time > 0);
}

void MarkovianCC::onACK(int ack, double receiver_timestamp __attribute((unused))) {
	int seq_num = ack - 1;

	// some error checking
	if ( unacknowledged_packets.count( seq_num ) > 1 ) {
		std::cout<<"Dupack: "<<seq_num<<std::endl; return; }
	if ( unacknowledged_packets.count( seq_num ) < 1 ) {
		std::cout<<"Unknown Ack!! "<<seq_num<<std::endl; return; }

	const PktInformation& pkt_info = unacknowledged_packets[seq_num];
	const double cur_time = current_timestamp();

	min_rtt = min(min_rtt, cur_time - pkt_info.sent_time);

	// update rtt_acked_ewma
	rtt_acked_ewma = alpha_rtt * (cur_time - pkt_info.sent_time) +
		(1.0 - alpha_rtt) * rtt_acked_ewma;

	update_sending_rate(pkt_info);

	assert (rtt_acked_ewma > 0);

	// delete this pkt and any unacknowledged pkts before this pkt
	for (auto x = unacknowledged_packets.begin();
		x != unacknowledged_packets.end();) {
		if(x->first > seq_num)
			break;
		if(x->first < seq_num && cur_time - x->second.sent_time
				> 3 * max(rtt_acked_ewma, rtt_unacked_ewma)) {
			++ num_pkts_lost;
			const auto &t = *x;
			++x;
			unacknowledged_packets.erase(t.first);
		}
		else if (x->first == seq_num) {
			const auto &t = *x;
			x++;
			unacknowledged_packets.erase(t.first);
		}
		else
			x++;
	}
	++ num_pkts_acked;
}

void MarkovianCC::onLinkRateMeasurement( double s_measured_link_rate __attribute((unused)) ) {
	assert( false );
}

void MarkovianCC::onPktSent(int seq_num) {
	// add to list of unacknowledged packets
	assert( unacknowledged_packets.count( seq_num ) == 0 );
	double cur_time = current_timestamp();
	unacknowledged_packets[seq_num] = {
		sending_rate,
		sending_rate_velocity,
		cur_time};

	// check if rtt_ewma is to be increased
	rtt_unacked_ewma = rtt_acked_ewma;
	int tmp_seq_num = -1;
	for (auto & x : unacknowledged_packets) {
		assert(tmp_seq_num < x.first);
		tmp_seq_num = x.first;
		double rtt_lower_bound = cur_time - x.second.sent_time;
		if (rtt_lower_bound <= rtt_unacked_ewma)
			break;
		rtt_unacked_ewma = alpha_rtt * rtt_lower_bound +
			(1.0 - alpha_rtt) * rtt_unacked_ewma;
	}
}
