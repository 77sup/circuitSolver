#include "include/solver.h"
#include "include/iscas89_parser.h"
#include "include/Time.h"
#include <string>
#include <fstream>
#include<time.h>

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
	auto Parser = 0;
	auto Structure = 0;
	auto Solver = 0;
	ElapsedTimer t(true);
	CircuitGraph graph;
	Iscas89Parser parser;
	if (!parser.parse(ifs, graph))
	{
		log_error() << "can't parse file" << argv[1];
		return 1;
	}
	Parser =  t.get_elapsed_us();

	srand(time(NULL));
	solver solver(graph);
	Structure =  t.get_elapsed_us();

	solver.CDCLsolver(graph);
	//std::cout<<"end function CDCLsolver"<<std::endl;
	//std::cout<<"number: "<<number<<std::endl;
	Solver = t.get_elapsed_us();
	std::cout << "the time of parser:   " << 1.0 * Parser/1000 << "  ms" << std :: endl;
	std::cout << "the time of structure:" << 1.0 * (Structure - Parser)/1000 << "  ms" << std :: endl;
	std::cout << "the time of solver:   " << 1.0 * (Solver - Structure)/1000 << "  ms" << std :: endl;
	return 0;
}
