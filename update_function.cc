#include "update_function.hh"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>

using namespace std;

double& UpdateFunction::func(double x, double y) {
	assert(0 <= x && x <= 1);
	assert(0 <= y && y <= 1);
	unsigned x_id = function.size() * x, y_id = function.size() * y;
	if (x == 1.0) -- x_id;
	if (y == 1.0) -- y_id;
	return function[x_id][y_id];
}

double UpdateFunction::func_der(double x, double y) {
	assert(0 <= x && x <= 1);
	assert(0 <= y && y <= 1);
	unsigned x_id = function.size() * x, y_id = function.size() * y;
	if (x == 1.0) -- x_id;
	if (y == 1.0) -- y_id;
	
	double n = function.size();
	
	double x_up_der = 0, x_down_der = 0;
	if (x_id < function.size() - 1)
		x_up_der = (function[x_id + 1][y_id] - function[x_id][y_id]) / n;
	if (x_id > 0)
		x_down_der = (function[x_id][y_id] - function[x_id - 1][y_id]) / n;
	double x_der = (x_up_der + x_down_der) / 2;
	if (x_id == function.size() - 1) x_der = x_down_der;
	if (x_id == 0) x_der = x_up_der;
	
	double y_up_der = 0, y_down_der = 0;
	if (y_id < function.size() - 1)
		y_up_der = (function[x_id][y_id + 1] - function[x_id][y_id]) / n;
	if (y_id > 0)
		y_down_der = (function[x_id][y_id] - function[x_id][y_id - 1]) / n;
	double y_der = (y_up_der + y_down_der) / 2;
	if (y_id == function.size() - 1) y_der = y_down_der;
	if (y_id == 0) y_der = y_up_der;

	if (std::isnan(y_der) || std::isinf(y_der) || std::isinf(x_der)) {
		cout << x_id << " " << y_id	<< " " << x << " " << y << " " << y_up_der << " " << y_down_der << endl << flush;
		cout << function[x_id][y_id - 1] << " " << function[x_id][y_id] << " " << function[x_id][y_id + 1] << endl << flush;
		cout << x_up_der << " " << x_down_der << " " << x_der << endl;
	}
	assert(!std::isnan(x_der));
	assert(!std::isnan(y_der));
	
	return x_der - y_der;		 	
}

bool UpdateFunction::consider(const vector<double>& lambdas) {
	const double alpha = -0.1;

	double sum = 0;
	for (auto x : lambdas)
		sum += x;
		
	double ideal = mu / (lambdas.size() + delta);
	
	double cur_dist = 0, next_dist = 0;
	for (auto x : lambdas) {
		cur_dist += UpdateFunction::square(x - ideal);
		next_dist += UpdateFunction::square(func(x, mu - sum) - ideal);
	}
	// if (next_dist <= 0.5 * cur_dist)
	// 	return false;

	for (auto & x : lambdas) {
		func(x, mu - sum) += alpha * (func(x, mu - sum) - ideal);
	}
	return true;
}

void UpdateFunction::generate_lambdas(vector<double>& lambdas) {
	static default_random_engine rand_generator;
	static uniform_real_distribution<double> lambda_distribution(0.0, 1.0);
	static uniform_int_distribution<int> n_distribution(2, max_n);
	
	unsigned n = n_distribution(rand_generator);
	lambdas.resize(n);
	double sum = 0;
	for (auto & x : lambdas) {
		x = lambda_distribution(rand_generator);
		sum += x;
	}
	sum /= lambda_distribution(rand_generator);
	for (auto & x : lambdas)
		x /= sum;
}

void UpdateFunction::train(double epsilon, unsigned max_iter) {
	vector<double> lambdas(max_n);

	const double alpha_frac_correct = 1.0/256.0;
	double frac_correct = 0;

	for (unsigned i = 0; i < max_iter; i++) {
		generate_lambdas(lambdas);

		frac_correct *= 1 - alpha_frac_correct;
		if (!consider(lambdas))
			frac_correct += alpha_frac_correct;

		if (frac_correct > 1 - epsilon) {
			cout << "Took " << i + 1 << " iterations to converge." << endl;
			return;
		}
	}
	cout << "Could not converge afer " << max_iter << " iterations.mak" << endl;
}

void UpdateFunction::write(string filename, int type) {
	ofstream out_file (filename, ios::out);
	out_file << type << " " << function.size() << endl;
	// Square format
	if (type == 1) {
		for (const auto & x : function) {
			for (const auto & y : x) {
				out_file << y << " ";
			}
			out_file << endl;
		}
	}
	// (x, y, z) format
	else if (type == 2){
		for (unsigned i = 0; i < function.size(); i++) {
			for (unsigned j = 0; j < function.size(); j++) {
				double x = 1.0 * i / function.size();
				double y = 1.0 * j / function.size();
				out_file << x << " " << y << " " << function[i][j] \
					<< " " << (x + y) / (delta + 1) << " " << x << endl; 
			}
		}
	}
	else
		cerr << "Write Failed: Unrecognized type " << type << endl;
	out_file.close ();
}

void UpdateFunction::read(string filename) {
	ifstream in_file (filename, ios::in);
	int type, size;
	in_file >> type >> size;
	
	function.resize(size);
	for (auto & x : function)
		x.resize(size, 0.0);

	if (type == 1)
		cerr << "Read Failed: Matrix format not yet supported" << endl;
	else if (type == 2) {
		while (!in_file.eof()) {
			double x, y, z, t1, t2;
			in_file >> x >> y >> z >> t1 >> t2;
			func(x + 0.1/size, y +0.1/size) = z;
		}
	}
}

void UpdateFunction::test(unsigned num_iter) {
	// Test if function is a (semi) contraction
	vector<double> lambdas(max_n);

	double mean_contraction = 0, var_contraction = 0;
	int num_correct = 0;

	for (unsigned i = 0; i < num_iter; i++) {
		generate_lambdas(lambdas);

		double sum = 0;
		for (auto x : lambdas)
			sum += x;
		double ideal = mu / (lambdas.size() + delta);
		double cur_dist = 0, next_dist = 0;
		for (auto x : lambdas) {
			cur_dist += UpdateFunction::square(x - ideal);
			next_dist += UpdateFunction::square(func(x, mu - sum) - ideal);
		}

		mean_contraction += next_dist / cur_dist;
		var_contraction += UpdateFunction::square(next_dist / cur_dist);

		if (next_dist / cur_dist < 0.9)
			++ num_correct;
	}
	mean_contraction /= num_iter;
	var_contraction /= num_iter;
	var_contraction -= UpdateFunction::square(mean_contraction);

	cout << "Contraction Test: " << "\n\t Mean: " << mean_contraction
		<< "\n\t Variance: " << var_contraction << "\n\t Perc. Correct: " 
		<< 100.0 * num_correct / num_iter << endl;

	// Test if function is an identity where required
	double mean_deviation = 0, var_deviation = 0;
	unsigned identity_limit = function.size() / (2.0 + delta);
	for (unsigned i = 0; i < identity_limit; i++) {
		double dev = function[i][i] - 1.0 * i / function.size();
		mean_deviation += abs(dev);
		var_deviation += UpdateFunction::square(dev);
	}
	mean_deviation /= identity_limit;
	var_deviation /= identity_limit;
	var_deviation -= UpdateFunction::square(mean_deviation);

	cout << "Identity Test: " << "\n\t Mean Deviation: " << mean_deviation \
		<< "\n\t Var Deviation: " << var_deviation << endl;
}
