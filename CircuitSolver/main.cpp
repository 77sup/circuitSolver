#include "include/solver.h"
#include "include/iscas89_parser.h"
#include "include/Time.h"
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
	auto Parser = 0;
	auto Structure = 0;
	auto Solver = 0;
	ElapsedTimer t(true);
	CircuitGraph m_graph;
	Iscas89Parser parser;
	if (!parser.parse(ifs, m_graph))
	{
		log_error() << "can't parse file" << argv[1];
		return 1;
	}
	CircuitGraph graph(m_graph);
	Parser =  t.get_elapsed_us();
	solver solver(graph);
	Structure =  t.get_elapsed_us();

	//solver.solve(graph);
	solver.CDCLsolver(graph);
	Solver = t.get_elapsed_us();
	std::cout << "the time of parser:   " << 1.0 * Parser/1000 << "  ms" << std :: endl;
	std::cout << "the time of structure:" << 1.0 * (Structure - Parser)/1000 << "  ms" << std :: endl;
	std::cout << "the time of solver:   " << 1.0 * (Solver - Structure)/1000 << "  ms" << std :: endl;
	for(int i=0;i<origin_cnf.size();i++)
	{
		for(int j=0;j<origin_cnf[i].size();j++) std::cout<<origin_cnf[i][j]<<" ";
		std::cout<<std::endl;
	}
	/*std::cout<<"output name:"<<std::endl;
	for(int i=0;i<graph.get_outputs().size();i++)
	{
		std::cout<<graph.get_outputs()[i]->num_name;
	}*/

	return 0;
}
