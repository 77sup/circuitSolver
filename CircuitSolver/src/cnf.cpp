#include"../include/cnf.h"
#include <iostream>
#include <string>
#include <cmath>

void cnf_bcp::initialize()
{
     //char c;   // store first character
  //std::string s; // dummy string
  /*c 3 variables, 6 clauses
  p cnf 3 6
  1 2 0
  1 -2 0
  3 2 0
  -3 1 0
  1 2 3 0
  -1 -2 0*/
  /*while (true)
  {
    std::cin >> c;
    if (c == 'c') // if comment
    {
      getline(std::cin, s); // ignore
    }
    else // else, if would be a p
    {
      std::cin >> s; // this would be cnf
      break;
    }
  }*/
  std::cin >> literal_count;
  std::cin >> clause_count;
  //initial
  formula.literals.clear();
  formula.literals.resize(literal_count, -1);
  formula.clauses.clear();
  formula.clauses.resize(clause_count);
  formula.literal_frequency.clear();
  formula.literal_frequency.resize(literal_count, 0);
  formula.literal_polarity.clear();
  formula.literal_polarity.resize(literal_count, 0);
  int literal;
  for (int i = 0; i < clause_count; i++) // iterate over the clauses
  {
    while (true) // while the ith clause gets more literals
    {
      std::cin>>literal;   //store the incoming literal value
      if (literal > 0) // variable has positive polarity
      {
        formula.clauses[i].push_back(literal); 
        formula.literal_frequency[literal - 1]++;  // increment frequency and polarity of the literal
        formula.literal_polarity[literal - 1]++;
      }
      else if (literal < 0) // variable has negative polarity
      {
        formula.clauses[i].push_back(literal); 
        formula.literal_frequency[-1 - literal]++;    // increment frequency and decrement polarity of the literal
        formula.literal_polarity[-1 - literal]--;
      }
      else
      {
        break; // read 0 means up to  this clause tail , so move to next clause
      }
    }
  }
}


int cnf_bcp::unit_propagate(Formula &f)
{
  bool unit_clause_found = false; //whether the current iteration found a unit clause
  if (f.clauses.size() == 0) return 1;//formula contains no clauses, it is vacuously satisfied
  do
  {
    unit_clause_found = false;
    for (int i = 0; i < f.clauses.size(); i++)  // iterate over the clauses in f
    {                             
      if (f.clauses[i].size() == 1)   //it is a unit clause
      {
        unit_clause_found = true;
        if(f.clauses[i][0]>0)
        {
            f.literals[f.clauses[i][0]-1]=1;
        }
        else if(f.clauses[i][0]<0)
        {
            f.literals[-1 * f.clauses[i][0]-1]=0;
        }
        f.literal_frequency[abs(f.clauses[i][0])-1] = -1;         // once assigned, reset the frequency to mark it closed
        int result = apply_transform(f, abs(f.clauses[i][0])-1);  // apply this change through f
        //if this caused the formula to be either satisfied or unsatisfied,
        if (result == -1)
        { // return the result flag
          return result;
        }
        break; // exit the loop to check for another unit clause from the start
      }
      else if (f.clauses[i].size() == 0) // if a given clause is empty
      {
        return 0;  // the formula is unsatisfiable in this branch
      }
    }
  } while (unit_clause_found);
  return 1; // if reached here, the unit resolution ended normally
}

int cnf_bcp::apply_transform(Formula &f, int literal_to_apply)
{
    int value_to_apply = f.literals[literal_to_apply]; //the value to apply,1 -if true, 0 - if false
  for (int i = 0; i < f.clauses.size(); i++)         // iterate over the clauses in f
  {
    for (int j = 0; j < f.clauses[i].size(); j++) // iterate over the variables in the clause
    {
      if ((pow(-1, value_to_apply)*(literal_to_apply + 1)) == f.clauses[i][j])   //oposite polarity
      {
        f.clauses[i].erase(f.clauses[i].begin() + j); // remove the literal from the clause
        j--;                                          // reset the iterator
        if (f.clauses[i].size() == 0)          // if the clause is empty, the formula is unsatisfiable currently
        {
          std::cout<<"  12345   "<<std::endl;
          return -1;         //unsatisfied,cnf_bcp fail
        }
        break;  // move to the next clause
      }
      else if (literal_to_apply + 1  == abs(f.clauses[i][j])) // same polarity
      {
        f.clauses.erase(f.clauses.begin() + i); // remove the clause from the list
        i--;                                    // reset iterator
        if (f.clauses.size() == 0)              // if all clauses have been removed, the formula is satisfied
        {
          return 1;         //satisfied
        }
        break; // move to the next clause
      }
    }
  }
  return 1; // if reached here, the function is exiting normally
}
void cnf_bcp::show(int result)
{
    if(result==1)
    {
        std::cout<<"cnf_bcp success:"<<formula.literals.size()<<std::endl;

        for(int i=0;i < formula.literals.size();i++)
        {
            std::cout << formula.literals[i]<<std::endl;
        }
    }
    else
    {
        std::cout<<"cnf_bcp fail"<<std::endl;
    }
}
void cnf_bcp::test()
{
    int result=unit_propagate(formula);
    show(result);
}
int cnf_bcp::cnf_BCP(Formula& f)
{
  return unit_propagate(f);
}