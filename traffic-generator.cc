#include "traffic-generator.hh"

using namespace std;

TrafficGenerator::TrafficGenerator(double s_mean_on_unit, double s_mean_off_unit, string traffic_params)
	:	_traffic_type(TrafficType::EXPONENTIAL_ON_OFF),
		_switch_type(SwitchType::TIME_SWITCHED),
		_traffic_params({ s_mean_off_unit, s_mean_on_unit, (unsigned int)-1 }),
		prng(global_PRNG()),
		senders()
{
	_traffic_params._on_off._mean_on_unit = s_mean_on_unit;
	_traffic_params._on_off._mean_off_unit = s_mean_off_unit;

	// parse traffic_params
	size_t start_pos = 0;
	while (start_pos < traffic_params.length()) {
		size_t end_pos = traffic_params.find(',', start_pos);
		if (end_pos == string::npos)
			end_pos = traffic_params.length();
		
		string arg = traffic_params.substr(start_pos, end_pos);
		if (arg == "exponential")
			_traffic_type = TrafficType::EXPONENTIAL_ON_OFF;
		else if (arg == "deterministic")
			_traffic_type = TrafficType::DETERMINISTIC_ON_OFF;
		else if (arg == "byte_switched"){
			_switch_type = SwitchType::BYTE_SWITCHED;
		}
		else if (arg.substr(0, 11) == "num_cycles=")
			_traffic_params._on_off.num_cycles = (unsigned int) \
				atoi(arg.substr(11).c_str());
		else 
			cout << "Unrecognised parameter: " << arg << endl;

		start_pos = end_pos + 1;
	}
}

bool TrafficGenerator::switch_state(
	int num_pkts_transmitted, 
	TickNum tick_num,
	SenderId sender_id) {

	// check if this sender_id has been seen previously
	if (senders.count(sender_id) == 0) {
		senders[sender_id] = make_tuple(_traffic_params._on_off._mean_off_unit, false, tick_num);
		cout << "Switching off for " << get<0>(senders[sender_id]) << " at " << tick_num << endl;
	}

	// sender is currently off
	if (get<1>(senders[sender_id]) == false) {
		if (tick_num - get<2>(senders[sender_id]) >= get<0>(senders[sender_id])) {
			if (_traffic_type == TrafficType::DETERMINISTIC_ON_OFF)
				get<0>(senders[sender_id]) = _traffic_params._on_off._mean_on_unit;
			else {
				Exponential on (1 / _traffic_params._on_off._mean_on_unit, prng);
				get<0>(senders[sender_id]) = on.sample();
			}
			get<1>(senders[sender_id]) = true;
			get<2>(senders[sender_id]) = tick_num;
			cout << "Switching on for " << get<0>(senders[sender_id]) << " at " << tick_num << endl;
		}
	}
	// sender is currently on
	else {
		double duration = (_switch_type == SwitchType::BYTE_SWITCHED) ? \
			double(num_pkts_transmitted) : \
			(tick_num - get<2>(senders[sender_id]));
		if (duration >= get<0>(senders[sender_id])) {
			if (_traffic_type == TrafficType::DETERMINISTIC_ON_OFF)
				get<0>(senders[sender_id]) = _traffic_params._on_off._mean_off_unit;
			else {
				Exponential off (1 / _traffic_params._on_off._mean_off_unit, prng);
				get<0>(senders[sender_id]) = off.sample();
			}
			get<1>(senders[sender_id]) = false;
			get<2>(senders[sender_id]) = tick_num;
			cout << "Switching off for " << get<0>(senders[sender_id]) << " at " << tick_num << endl;
		}
	}
	return get<1>(senders[sender_id]);
}

TickNum TrafficGenerator::num_ticks_to_switch_off(SenderId sender_id) {
	assert(get<1>(senders[sender_id]) == false);
	return get<0>(senders[sender_id]);
}