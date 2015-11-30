#include <random>
#include <string>
#include <vector>

class UpdateFunction {
	const unsigned max_n = 32;

	// lambda_i major, mu-lambda_i minor
	std::vector <std::vector <double> > function;
	double mu;
	double delta;
	
	static double square(double x) { return x*x; }
	double func_der(double x, double y);

	void generate_lambdas(std::vector<double>& lambdas);
	
	bool consider(const std::vector<double>& lambdas);
	
public:
	UpdateFunction(int domain_resolution)
	:	function(domain_resolution, std::vector <double>(domain_resolution, 0)),
		mu(1.0),
		delta(1.0)
	{}

	UpdateFunction(std::string filename)
	:	function(),
		mu(1.0),
		delta(1.0)
	{ 
		read(filename);
	}
	
	double& func(double x, double y);
	
	// Train (using gradient descent) the function.
	//
	// Runs until the error rate falls below epsilon or until max_iter
	// iterations are over
	void train(double epsilon, unsigned max_iter=1e6);

	// Write function to file in human readable format given filename
	//
	// If type == 1, prints in matrix form, if type == 2 prints in 
	// x, y, z form with extra columns for visualization
	void write(std::string filename, int type);

	// Read function from file written to by 'write'
	void read(std::string filename);

	// Performs tests to chech the validity of the function and prints
	// the test results to stdout
	void test(unsigned num_iter);
};
