#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include"../include/cnf.h"
#include <vector>
#include<queue>
#include <cmath>

int solver::unit_propagate(cnf &f, std::queue<int>&bcp_que,int reason_line_name)
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
            lines_status_num.at(f.clauses[i][0])=1;
            bcp_que.push(abs(f.clauses[i][0]));
           
        }
        else if(f.clauses[i][0]<0)
        {
            lines_status_num.at(abs(f.clauses[i][0]))=0;
            bcp_que.push(abs(f.clauses[i][0]));
        }
       // f.literal_frequency[abs(f.clauses[i][0])-1] = -1;         // once assigned, reset the frequency to mark it closed
        int result = apply_transform(f, bcp_que, abs(f.clauses[i][0]));  // apply this change through f
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
int solver::apply_transform(cnf &f, std::queue<int>&bcp_que,int reason_line_name)
{
     int value_to_apply = lines_status_num.at(reason_line_name); //the value to apply,1 -if true, 0 - if false
  
  for (int i = 0; i < f.clauses.size(); i++)         // iterate over the clauses in f
  {
    for (int j = 0; j < f.clauses[i].size(); j++) // iterate over the variables in the clause
    {
      if ((pow(-1, value_to_apply)*reason_line_name) == f.clauses[i][j])   //oposite polarity
      {
        f.clauses[i].erase(f.clauses[i].begin() + j); // remove the literal from the clause
        j--;                                          // reset the iterator
        if (f.clauses[i].size() == 0)          // if the clause is empty, the formula is unsatisfiable currently
        {
          std::cout<<"empty clause confilict: "<<
          reason_line_name<<"  assigned: "<<value_to_apply<<std::endl;
          return -1;         //unsatisfied, cnf_bcp fail
        }

        break;  // move to the next clause
      }
      else if (reason_line_name == abs(f.clauses[i][j])) // same polarity
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