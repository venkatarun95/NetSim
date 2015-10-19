#ifndef RAND_GEN_HH
#define RAND_GEN_HH

#include <chrono>
#include <random>

class RandGen {
	unsigned seed;
	std::default_random_engine generator;

public:
	RandGen()
	:	seed(std::chrono::system_clock::now().time_since_epoch().count()),
		generator(seed)
	{}

	double exponential(double lambda);
	double uniform(double min, double max);
};

extern RandGen rand_gen;

#endif