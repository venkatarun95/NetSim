#include "rand-gen.hh"

RandGen rand_gen;

using namespace std;

double RandGen::exponential(double lambda) {
	exponential_distribution<double> distr(lambda);
	return distr(generator);
}

double RandGen::uniform(double min, double max) {
	uniform_real_distribution<double> distr(min, max);
	return distr(generator);	
}
