#ifndef MARKOVIANCC_HH
#define MARKOVIANCC_HH

// to decide the source of timestamp
#define SIMULATION_MODE

#include <chrono>
#include <functional>
#include <limits>
#include <map>
#include <queue>
#include <vector>

#include "ccc.hh"
#include "exponential.hh"
#include "utilities.hh"

class MarkovianCC : public CCC {
	double delta;

	// some adjustable parameters
	const double alpha_rtt = 1.0/2.0;
	const double alpha_intersend = 1.0/2.0; //1.0/2.0;
	const double alpha_update = 1.0; //1.0/2.0;
	const double initial_intersend_time = 10.0;
	// set of all unacked pkts Format: (seq_num, sent_timestamp)
	//
	// Note: a packet is assumed to be lost if a packet with a higher
	// sequence number is acked. This set contains only the packets
	// which are NOT lost
	std::map<int, double, std::function<bool(const int&, const int&)> > unacknowledged_packets;

	double min_rtt;

	TimeEwma mean_sending_rate;

	TimeEwma rtt_acked_ewma;  // estimated using only acked packets
	TimeEwma rtt_unacked_ewma; // unacked packets are also considered
	TimeEwma intersend_ewma;
	// send time of previous ack. Used to calculate intersend time
	double prev_ack_sent_time;

	// cur_tick is measured relative to this
	std::chrono::high_resolution_clock::time_point start_time_point;

	// a pkt is considered lost if a gap appears in acks
	unsigned int num_pkts_lost;
	// to calculate % lost pkts
	unsigned int num_pkts_acked;

	// Variables for expressing explicit utility functions

	const std::vector<double> delta_classes = {0.1, 0.2, 0.5, 1, 2, 3, 4, 6, 8};
	int cur_delta_class;
	// Last time the delta was updated
	int last_delta_update_time;

	#ifdef SIMULATION_MODE
	// current time to be used during simulation
	double cur_tick;
	#endif

	// return a timestamp in milliseconds
	double current_timestamp();

	// update intersend time based on rtt ewma and intersend ewma
	void update_intersend_time();

	// Update the delta to express explicit utility functions
	void update_delta();

public:
	MarkovianCC( double s_delta )
	: 	CCC(),
		delta( s_delta ),
		unacknowledged_packets([](const int& x, const int& y){return x > y;}),
		min_rtt(),
		mean_sending_rate(alpha_update),
		rtt_acked_ewma(alpha_rtt),
		rtt_unacked_ewma(alpha_rtt),
		intersend_ewma(alpha_intersend),
		prev_ack_sent_time(),
		start_time_point(),
		num_pkts_lost(),
		num_pkts_acked(),
		cur_delta_class(0),
		last_delta_update_time(0),
		#ifdef SIMULATION_MODE
		cur_tick()
		#endif
	{}

	// callback functions for packet events
	virtual void init() override;
	virtual void onACK(int ack, double receiver_timestamp) override ;
	virtual void onPktSent(int seq_num) override ;
	virtual void onTimeout() override { std::cerr << "Ack timed out!\n"; }
	virtual void onLinkRateMeasurement( double s_measured_link_rate ) override;

	#ifdef SIMULATION_MODE
	void set_timestamp(double s_cur_tick) {cur_tick = s_cur_tick;}
	#endif

	// int get_delta_class() { return cur_delta_class; }
	int get_delta_class() { return (delta == 0.1)?0:1; }
};

#endif
