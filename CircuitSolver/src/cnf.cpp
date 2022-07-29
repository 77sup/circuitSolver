#include"../include/cnf.h"
#include <iostream>
#include <string>
#include <cmath>

void cnf::initialize()
{
  std::cout<<"please input the number of clauses:"<<std::endl;
  int clause_count;
  std::cin>>clause_count;
  std::cout<<"please input all clause:"<<std::endl;
  int literal;
  std::vector<int> clause;
  for (int i = 0; i < clause_count; i++) // iterate over the clauses
  {
    clause.clear();
    while(true)
    {
      std::cin>>literal;
      if(literal==0) break;
      clause.push_back(literal);
    }
    clauses.push_back(clause);
   
  }
}




