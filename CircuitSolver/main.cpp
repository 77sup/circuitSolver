#include "include/solver.h"
#include "include/iscas89_parser.h"
#include "include/log.h"
#include "../include/cnf.h"

#include <string>
#include <fstream>

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		log_error() << "no input file specified";
		return 1;
	}
	std::ifstream ifs(argv[1]);
	if (!ifs.good())
	{
		log_error() << "can't open file" << argv[1];
		return 1;
	}
	CircuitGraph graph;
	Iscas89Parser parser;
	if (!parser.parse(ifs, graph))
	{
		log_error() << "can't parse file" << argv[1];
		return 1;
	}

	//solver CircuitSolver(graph);    //有参构造实例化对象
	//CircuitSolver.solve(graph);
	solver solver(graph);
	solver.solve(graph);
	//solver.testmap();
	
	return 0;
}
