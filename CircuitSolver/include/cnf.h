#include "circuit_graph.h"
#include<vector>
#include <iostream>
#include <string>
#ifndef CNF_H
#define CNF_H


class Formula
{
public:
  std::vector<int> literals;          // stores the value assigned to each variable, where -1 - unassigned,  0 - true,  1 - false
  std::vector<int> literal_frequency; // store the number of occurrences of each literal
  std::vector<int> literal_polarity;  // store the difference in number of occurrences with positive and negative polarity of each literal
  // vector<int> is one clause, if variable n is positive polarity, then 2n is stored; 
  std::vector<std::vector<int>> clauses;  //if variable n is negative polarity, then 2n+1 is stored; n is assumed to be zero indexed
  Formula() {}
  Formula(const Formula &f) // copy constructor for copying a formula - each member is copied over
  {
    literals = f.literals;
    clauses = f.clauses;
    literal_frequency = f.literal_frequency;
    literal_polarity = f.literal_polarity;
  }
};

class cnf_bcp
{
public:
  Formula formula;          // the initial formula given as input

  void initialize();
  cnf_bcp() {}
  void test();
  int cnf_BCP(Formula &);
  
private:                     
  int literal_count;                   // the number of variables in the formula
  int clause_count;                    // the number of clauses in the formula
  int unit_propagate(Formula &);       // performs unit propagation
  int apply_transform(Formula &, int); // applies the value of the literal in
  void show(int );
};












#endif