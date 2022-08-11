#include "circuit_graph.h"
#include<vector>
#include <iostream>
#include <string>
#include<queue>
#ifndef CNF_H
#define CNF_H


class cnf
{
public:
  std::vector<int> literal_frequency; // store the number of occurrences of each literal
  std::vector<int> literal_polarity;  // store the difference in number of occurrences with positive and negative polarity of each literal
  std::vector<std::vector<int>> clauses;  
  cnf() {}
  cnf (const cnf &f)   // copy constructor for copying a formula
  {
    clauses = f.clauses;
    literal_frequency = f.literal_frequency;
    literal_polarity = f.literal_polarity;
  }
  void initialize();
};





#endif