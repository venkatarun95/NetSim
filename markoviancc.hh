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

	// Some adjustable parameters

	static constexpr double alpha_rtt = 1.0;
	static constexpr double initial_intersend_time = 10.0;
	// Oscillator parameters
	static constexpr double spring_k = 1.0;
	static constexpr double damping = 2.0;
	static constexpr double mass = 1.0;
	// The \Delta t used for updating position and velocity
	static constexpr double dt = 0.01;

	struct PktInformation {
		// To store data about the time the acket was sent
		double sending_rate;
		double sending_rate_velocity;
		double sent_time;
	};

	// Set of all unacked pkts Format: (seq_num, sent_timestamp)
	//
	// Note: a packet is assumed to be lost if a packet with a higher
	// sequence number is acked. This set contains only the packets
	// which are NOT lost. This condition is currently under revision.
	std::map<int, PktInformation, std::function<bool(const int&, const int&)> >
		unacknowledged_packets;

	// MarkovianCC state variables

	double min_rtt;
	// Estimated using only acked packets
	double rtt_acked_ewma;
	// Unacked packets are also considered
	double rtt_unacked_ewma;
	double sending_rate;
	// First order rate of change of sending rate.
	double sending_rate_velocity;

	// cur_tick is measured relative to this
	std::chrono::high_resolution_clock::time_point start_time_point;

	// a pkt is considered lost if a gap appears in acks
	unsigned int num_pkts_lost;
	// to calculate % lost pkts
	unsigned int num_pkts_acked;

	#ifdef SIMULATION_MODE
	// current time to be used during simulation
	double cur_tick;
	#endif

	// return a timestamp in milliseconds
	double current_timestamp();

	// update intersend time based on rtt ewma and intersend ewma
	void update_sending_rate(const PktInformation& pkt_info);

public:
	MarkovianCC( double s_delta )
	: 	CCC(),
		delta( s_delta ),
		unacknowledged_packets([](const int& x, const int& y){return x < y;}),
		min_rtt(),
		rtt_acked_ewma(),
		rtt_unacked_ewma(),
		sending_rate(),
		sending_rate_velocity(0),
		start_time_point(),
		num_pkts_lost(),
		num_pkts_acked(),
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
