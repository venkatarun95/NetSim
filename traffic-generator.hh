#ifndef TRAFFIC_GENERATOR_HH
#define TRAFFIC_GENERATOR_HH

#include "exponential.hh"
#include "random.hh"
#include "utilities.hh"

#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>

class TrafficGenerator {
	enum TrafficType {EXPONENTIAL_ON_OFF, DETERMINISTIC_ON_OFF};
	enum SwitchType {TIME_SWITCHED, BYTE_SWITCHED};

	TrafficType _traffic_type;
	SwitchType _switch_type;

	union TrafficParams {
		struct{
			double _mean_off_unit;
			double _mean_on_unit;
			unsigned int num_cycles;
		} _on_off;
	} _traffic_params;

	PRNG prng;

	// For each sender, stores the next unit of time to remain on or
	// off, its current state (true - on, false - off) and the last 
	// time its state was changed
	std::unordered_map< SenderId, std::tuple<double, bool, TickNum> > senders;

public:
	// Takes num bytes transmitted, ticks transmitted and sender id.
	// Returns true iff that sender should be switched on.
	//
	// If a previously unseen sender id is given, it records it as a
	// new sender. It never deletes previously recorded sender, so be
	// careful about memory
	bool switch_state(
		int num_pkts_transmitted, 
		TickNum tick_num, 
		SenderId sender_id);

	// Returns the number of ticks to switch off
	TickNum num_ticks_to_switch_off(SenderId sender_id);

	TrafficGenerator(double s_mean_on_unit, double s_mean_off_unit, std::string traffic_params);
};

#endif