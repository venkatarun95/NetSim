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

	mean_sending_rate.reset();

	rtt_acked_ewma.reset();
	rtt_unacked_ewma.reset();
	intersend_ewma.reset();
 	_intersend_time = initial_intersend_time;
	prev_ack_sent_time = 0.0;

	_the_window = numeric_limits<double>::max();
	_timeout = 1000;

	num_pkts_lost = num_pkts_acked = 0;

	start_time_point = std::chrono::high_resolution_clock::now();
}

void MarkovianCC::update_delta() {
	double cur_time = current_timestamp();
	if (cur_time < last_delta_update_time + 100)
		return; // update every few milliseconds

	double rtt_ewma = max(rtt_acked_ewma, rtt_unacked_ewma);
	if (rtt_ewma > 12)
		cur_delta_class ++;
	else
		cur_delta_class --;
	cur_delta_class = max(0, min((int)delta_classes.size() - 1, cur_delta_class));
	delta = delta_classes[cur_delta_class];
	cout << "New delta: " << delta << " " << rtt_unacked_ewma << endl;
}

void MarkovianCC::update_intersend_time() {
	double cur_time = current_timestamp();
	if (!intersend_ewma.is_valid())
	 	return;

  // update_delta();

	double rtt_ewma = max(rtt_unacked_ewma, rtt_acked_ewma);

	double tmp_old_intersend_time = _intersend_time;

	// Utility = throughput / delay^\delta
	// double tmp_spare_rate = 1.0 / (rtt_ewma - min_rtt) + 1/intersend_ewma;
	// _intersend_time = 2*min_rtt / (delta+1 + 2*min_rtt*tmp_spare_rate -
	// 	sqrt((delta+1)*(delta+1) + 4*delta*min_rtt*tmp_spare_rate));

	// Utility = throughput / (queuing delay)^\delta
	// double new_sending_rate = (1.0/(rtt_ewma - min_rtt) + 1.0/intersend_ewma) / (delta + 1);
	double spare_rate = 1.0 / (rtt_ewma - min_rtt);
	double new_sending_rate = spare_rate / delta;


	if (num_pkts_acked < 10) {
	 	_intersend_time = max(1.0 / new_sending_rate, tmp_old_intersend_time); // to avoid bursts due tp min_rtt updates
	 	mean_sending_rate.update(new_sending_rate, cur_time / min_rtt);
	 	return;
	}

	if (new_sending_rate < 1.0 / initial_intersend_time)
		new_sending_rate = 1.0 / initial_intersend_time;
	// double alpha = (new_sending_rate - 1.0/intersend_ewma) / max(new_sending_rate,
		// 1.0 / intersend_ewma);
	// alpha *= alpha;
	// alpha = min(1.0, alpha);
	// cout << alpha << endl;
	// new_sending_rate = alpha * new_sending_rate + (1.0 - alpha) / intersend_ewma;

	// cout << "B " << cur_time << " " << new_sending_rate << " " << 1.0 / intersend_ewma << endl;
	// const double update_alpha = 1.0 / 32.0;
	// new_sending_rate = update_alpha * new_sending_rate + (1.0 - update_alpha) / intersend_ewma;

	mean_sending_rate.update(new_sending_rate, cur_time / min_rtt);

	_intersend_time = 1/mean_sending_rate;
	// _intersend_time = 1.0 / rand_gen.exponential(mean_sending_rate);

	assert(_intersend_time > 0);
}

void MarkovianCC::onACK(int ack, double receiver_timestamp __attribute((unused))) {
	int seq_num = ack - 1;

	// some error checking
	if ( unacknowledged_packets.count( seq_num ) > 1 ) {
		std::cout<<"Dupack: "<<seq_num<<std::endl; return; }
	if ( unacknowledged_packets.count( seq_num ) < 1 ) {
		std::cout<<"Unknown Ack!! "<<seq_num<<std::endl; return; }

	double sent_time = unacknowledged_packets[seq_num];
	double cur_time = current_timestamp();

	min_rtt = min(min_rtt, cur_time - sent_time);

	// update rtt_acked_ewma
	rtt_acked_ewma.update((cur_time - sent_time),
		cur_time / min_rtt);
	rtt_acked_ewma.round();

	assert (rtt_acked_ewma > 0);

	if (prev_ack_sent_time < sent_time) { //don't take reasing from reordered packets
	// update intersend_ewma
		if (prev_ack_sent_time != 0.0) {
			// intersend_ewma.update((1.0 / 128.0) * (sent_time - prev_ack_sent_time) +
			// 		(1.0 - 1.0 / 128.0) * intersend_ewma,
			// cur_time / min_rtt);
			intersend_ewma.update(sent_time - prev_ack_sent_time, cur_time / min_rtt);
			// intersend_ewma.round();
			update_intersend_time();
		}
		prev_ack_sent_time = sent_time;
	}
	else
		cout << "Reordering!" << endl;

	// delete this pkt and any unacknowledged pkts before this pkt
	for (auto x = unacknowledged_packets.begin();
		x != unacknowledged_packets.end();) {
		if(x->first > seq_num)
			break;
		if(x->first < seq_num && \
				cur_time - x->second > 3 * max(rtt_acked_ewma, rtt_unacked_ewma)) {
			++ num_pkts_lost;
			const auto &t = *x;
			++x;
			unacknowledged_packets.erase(t.first);
			//cout << "Lost: " << seq_num << " " << x->first << " " << cur_time - sent_time << " " << _intersend_time << endl;
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

	// if (_the_window < numeric_limits<int>::max())
	// 	_the_window = numeric_limits<int>::max();
}

void MarkovianCC::onLinkRateMeasurement( double s_measured_link_rate __attribute((unused)) ) {
	assert( false );
}

void MarkovianCC::onPktSent(int seq_num) {
	// add to list of unacknowledged packets
	assert( unacknowledged_packets.count( seq_num ) == 0 );
	double cur_time = current_timestamp();
	unacknowledged_packets[seq_num] = cur_time;

	// check if rtt_ewma is to be increased
	rtt_unacked_ewma = rtt_acked_ewma;
	for (auto & x : unacknowledged_packets) {
		double rtt_lower_bound = cur_time - x.second;
		if (rtt_lower_bound <= rtt_unacked_ewma)
			break;
		rtt_unacked_ewma.update(rtt_lower_bound, cur_time / min_rtt);
	}
	rtt_unacked_ewma.round();
}
