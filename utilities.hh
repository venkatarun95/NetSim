#ifndef UTILITIES_HH
#define UTILITIES_HH

#include <cassert>
#include <cmath>

// supports +, -, reciprocal and comparison. Assume 1 tick = 1 ms
typedef double TickNum;
// supports comparison only. 
typedef unsigned int FlowId;
// supports equality comparison only.
typedef unsigned int SenderId;

class TimeEwma {
	double ewma;
	double denominator;
	double alpha;
	double last_update_timestamp;

public:
	// lower the alpha, slower the moving average
	TimeEwma(const double s_alpha)
	:	ewma(),
		denominator(),
		alpha(1.0 - s_alpha),
		last_update_timestamp()
	{
		assert(alpha < 1.0);
	}

	void reset();
	void update(double value, double timestamp);
	void add(double value) {ewma += value;}
	void round() {ewma = int(ewma*100000) / 100000.0;}
	void force_set(double value, double timestamp);

	operator double() const {return ewma;}
	// true if update has been called atleast once
	bool is_valid() const {return denominator != 0.0;}
};

#endif