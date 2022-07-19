#include "../include/solver.h"
#include "../include/circuit_graph.h"
#include <vector>
#include<queue>
solver::solver(const CircuitGraph &graph)
{
    // std::vector<std::string> noPI_lines_name; // store no-PIs
    std::vector<int> noPO_lines_name;  // store no-PIs
    std::vector<int> output;          // store PIs
    std::cout<<"the number of all lins:";
    std::cout << graph.get_name_to_line().size() << std::endl;
    //std::cout<<"graph.get_lines().size():"<<graph.get_lines().size()<<std::endl;
    for (int i = 0; i < graph.get_lines().size(); i++)
    {
        if (graph.get_lines()[i].is_output)
        {
            output.push_back(graph.get_lines()[i].num_name);
        }
        else
        {
            noPO_lines_name.push_back(graph.get_lines()[i].num_name);
        }
        //std::cout<<"graph.get_lines()[i].num_name"<<graph.get_lines()[i].num_name<<std::endl;
    }
    std::cout<<"noPO_lines_name.size():  "<<noPO_lines_name.size()<<std::endl;
    std::cout<<"output.size():  "<<output.size()<<std::endl;
    //std::cout<<"graph.get_name_to_line().size():  "<<graph.get_name_to_line().size()<<std::endl;
    for (int i = 0; i < noPO_lines_name.size(); i++)
    {
        //std::cout<<noPI_lines_name[i]<<std::endl;
        lines_status_num.insert(std::make_pair(noPO_lines_name[i], -1));
        // find PIs to store
        if (!graph.get_name_to_line().at(noPO_lines_name[i])->source)
        {
            the_name_of_input_line.push_back(noPO_lines_name[i]);
        }
    }
    // print the_name_of_input_line
    /* for (int i = 0; i < the_name_of_input_line.size(); i++)
    {
        std::cout << "the_name_of_input_line:" << the_name_of_input_line[i] << std::endl;
    } */
    // assign POs to 1
    for (int i = 0; i < output.size(); i++)
    {
        lines_status_num.insert(std::make_pair(output[i], 1));
    }
    // test,print line's assign status
    /* std::cout << std::endl;
    for (auto it = lines_status.begin(); it != lines_status.end(); it++)
    {
        std::cout << "the_status_of_line:" << (*it).first << "    " << (*it).second << std::endl;
    } */
    // according to fan_outs numbers to order(max---min)
    int change;
    for (int i = 0; i < noPO_lines_name.size() - 1; i++)
    {
        for (int j = i + 1; j < noPO_lines_name.size(); j++)
        {
            if (graph.get_name_to_line().at(noPO_lines_name[i])->destination_gates.size() < graph.get_name_to_line().at(noPO_lines_name[j])->destination_gates.size())
            {
                change = noPO_lines_name[i];
                noPO_lines_name[i] = noPO_lines_name[j];
                noPO_lines_name[j] = change;
            }
        }
    }
    // store ordered noPO_lines
    for (int i = 0; i < noPO_lines_name.size(); i++)
    {
        sort_destination_gates.push_back(noPO_lines_name[i]);
    }
    // test
    /* for (int i = 0; i < sort_destination_gates.size(); i++)
    {
        std::cout << "sort_line_name:" << sort_destination_gates[i] << " num:  " << graph.get_name_to_line().at(sort_destination_gates[i])->destination_gates.size() << std::endl;
    } */
}
// choose a line to assign(decision),according to ordered fan_outs numbers
int solver::FindDecisionTarget(std::unordered_map<int, int> &lines_status_num)
{
    int Target;
    for (int i = 0; i < sort_destination_gates.size(); i++)
    {
        if (lines_status_num[sort_destination_gates[i]] == -1)
        {
            Target = sort_destination_gates[i];
            return Target;
        }
    }
    return -1;
}
void solver::test(const CircuitGraph &graph)
{
    this->lines_status_num.at(graph.get_lines()[2].num_name) = 0;
    std::cout << "Decision line name:   " << graph.get_lines()[2].name << std::endl;
    std::cout << "Decision line value:   " << lines_status_num.at(graph.get_lines()[2].num_name)<< std::endl;
    //std::vector<Gate *> line_connection_gates;
    std::cout<<BCP(graph,graph.get_lines()[2].num_name)<<std::endl;
    for (int i = 0; i < graph.get_lines().size(); i++)
    {
        std::cout << "line name:" << graph.get_lines()[i].name << "   assigned as:" << lines_status_num.at(graph.get_lines()[i].num_name)<< std::endl;
    }
}
void solver::solve(const CircuitGraph& graph)
{
    for(int i=0;i<graph.get_outputs().size();i++)
    {
        BCP(graph,graph.get_outputs()[i]->num_name);
        //std::cout<<BCP(graph,graph.get_outputs()[i]->name)<<std::endl<<std::endl;
    }
    int dpll_result=DPLL(graph,-1);
    if(!dpll_result) show_result(graph,0);
}
int solver::DPLL(const CircuitGraph& graph,int decision_line ) //return 0---unsat;return 1---sat,but this not related to solver's solution
{
    int bcp_result=BCP(graph,decision_line);
    if(bcp_result==0)  return 0;
    int next_decision_line=FindDecisionTarget(lines_status_num);
    if(next_decision_line==-1)   //all nodes were assigned
    {
        show_result(graph,1);
        return 1;  //solution is SAT
    }
    for(int i=0;i<2;i++)  //Traverse two child nodes
    {
        solver CircuitSolver=*this;  //kao bei yi ge duixiang
        CircuitSolver.lines_status_num.at(next_decision_line)=i; //update kao bei duixiang information
        int DPLL_result=CircuitSolver.DPLL(graph,next_decision_line);  
        if(DPLL_result) return 1;   //solution is SAT
    }
    return 0;  //solution is UNSAT
}

int solver::BCP(const CircuitGraph &graph,int decision_line)
{
    if(decision_line==-1) return 1;
    std::queue<int>bcp_que;              
    bcp_que.push(decision_line);
    int head_line_que=bcp_que.front();
    //SingleGateReasoning(Gate *current_gate, std::string reason_line_name);
    std::vector<Gate*> line_connection_gates;  
    while (!bcp_que.empty())
    {
        line_connection_gates.clear();
        //add the source gate
        if(graph.get_name_to_line().at(head_line_que)->source)
        {
            line_connection_gates.push_back(graph.get_name_to_line().at(head_line_que)->source);
        }
        //add the back gates
        for(auto it=graph.get_name_to_line().at(head_line_que)->destination_gates.begin();it!=graph.get_name_to_line().at(head_line_que)->destination_gates.end();it++)
        {
            line_connection_gates.push_back(*it);
        }
        for(int j=0;j<line_connection_gates.size();j++)
        {
            if(!SingleGateReasonBoost(line_connection_gates[j], bcp_que, head_line_que)) return 0; //SingleGateReasoning fail
        }
        bcp_que.pop();
        head_line_que=bcp_que.front();
    }
    return 1; 
}

void solver::show_result(const CircuitGraph& graph, int dpll_result)
    {
        if(dpll_result)
        {
            std::cout<<"SAT"<<std::endl;
            for(int i=0;i<graph.get_lines().size();i++){
                int line_name=graph.get_lines()[i].num_name;
                std::cout<<line_name<<"  "<<lines_status_num.at(line_name)<<std::endl;
            }
        }
        else
            std::cout<<"UNSAT"<<std::endl;
    }
//design for multiple inputs gates
bool solver::SingleGateReasonBoost(Gate* current_gate,std::queue<int>&bcp_que, int reason_line_name)
{
    Gate::Type GateType = current_gate->get_type();
    std::vector<std::pair<int, int>> all_lines_current_gate;
    for(int i=0;i<current_gate->get_inputs().size();i++)
    {
        int input=current_gate->inputs()[i]->num_name;
        all_lines_current_gate.push_back(std::make_pair(input,lines_status_num.at(input)));
    }
    all_lines_current_gate.push_back(std::make_pair(current_gate->get_output()->num_name,lines_status_num.at(current_gate->get_output()->num_name)));
    int number_lineOfGate=all_lines_current_gate.size();
    //respectively record input line's 0 1 x number 
    int input_line_status[3]={0,0,0};
    for(int i=0;i<number_lineOfGate-1;i++)
    {
        if(all_lines_current_gate[i].second==0)
        {
            input_line_status[0]++;
        }
        else if(all_lines_current_gate[i].second==1)
        {
            input_line_status[1]++;
        }
        else{
            input_line_status[2]++;
        }
    }
    int output_line_status=all_lines_current_gate[number_lineOfGate-1].second;
    switch (GateType)
    {
    case Gate::Type::And:
    {
        if((output_line_status==0&&input_line_status[1]==number_lineOfGate-1)||(output_line_status==1&&input_line_status[0]>0))
        {
            return false;  //conflict
        }
        else if(output_line_status==0&&input_line_status[2]==1&&input_line_status[1]==number_lineOfGate-2)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=0;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if(output_line_status==1&&input_line_status[2]>0)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }

        }
        else if(output_line_status==-1&&input_line_status[0]>0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&input_line_status[1]==number_lineOfGate-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else
        {
            return true; //nothing can be reasoned
        }
    }
    case Gate::Type::Nand:
    {
        if((output_line_status==1&&input_line_status[1]==number_lineOfGate-1)
        ||(output_line_status==0&&input_line_status[0]>0))
        {
            return false;  //conflict
        }
        else if(output_line_status==1&&input_line_status[2]==1&&input_line_status[1]==number_lineOfGate-2)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=0;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if(output_line_status==0&&input_line_status[2]>0)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }
        }
        else if(output_line_status==-1&&input_line_status[0]>0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&input_line_status[1]==number_lineOfGate-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else
        {
            return true; //nothing can be reasoned
        }
    case Gate::Type::Or:
    {
        if((output_line_status==0&&input_line_status[1]>0)||
        (output_line_status==1&&input_line_status[0]==number_lineOfGate-1))
        {
            return false;  //conflict
        }
        else if(output_line_status==0&&input_line_status[2]>0)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=0;
                    bcp_que.push(all_lines_current_gate[i].first);
                }
            }
            return true;
        }
        else if(output_line_status==1&&input_line_status[0]==number_lineOfGate-2&&input_line_status[2]==1)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }

        }
        else if(output_line_status==-1&&input_line_status[0]==number_lineOfGate-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&input_line_status[1]>0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Nor:
    {
        if((output_line_status==1&&input_line_status[1]>0)
        ||(output_line_status==0&&input_line_status[0]==number_lineOfGate-1))
        {
            return false;  //conflict
        }
        else if(output_line_status==0&&input_line_status[0]==number_lineOfGate-2&&input_line_status[2]==1)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=1;
                    bcp_que.push(all_lines_current_gate[i].first);
                    return true;
                }
            }

        }
        else if(output_line_status==1&&input_line_status[2]>0)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    lines_status_num.at(all_lines_current_gate[i].first)=0;
                    bcp_que.push(all_lines_current_gate[i].first);
                }
            }
                    return true;
        }
        else if(output_line_status==-1&&input_line_status[0]==number_lineOfGate-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&input_line_status[1]>0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Xor:  //two inputs
    {
        if((output_line_status==0&&input_line_status[0]>0&&input_line_status[1]>0)||
        (output_line_status==1&&(input_line_status[0]==2||input_line_status[1]==2)))
        {
            return false;
        }
        else if(output_line_status==0&&input_line_status[2]==1)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    if(input_line_status[0]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if(input_line_status[1]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=1;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }

        }
        else if(output_line_status==1&&input_line_status[2]==1)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    if(input_line_status[0]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=1;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if(input_line_status[1]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }

        }
        else if(output_line_status==-1&&input_line_status[0]>0&&input_line_status[1]>0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&(input_line_status[0]==2||input_line_status[1]==2))
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else
        {
            return true;
        }
        
    }
    case Gate::Type::Xnor:  //two inputs
    {
        if((output_line_status==1&&input_line_status[0]>0&&input_line_status[1]>0)||
        (output_line_status==0&&(input_line_status[0]==2||input_line_status[1]==2)))
        {
            return false;
        }
        else if(output_line_status==1&&input_line_status[2]==1)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    if(input_line_status[0]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if(input_line_status[1]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=1;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }

        }
        else if(output_line_status==0&&input_line_status[2]==1)
        {
            for(int i=0;i<number_lineOfGate-2;i++)
            {
                if(all_lines_current_gate[i].second==-1)
                {
                    if(input_line_status[0]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=1;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                    if(input_line_status[1]>0)
                    {
                        lines_status_num.at(all_lines_current_gate[i].first)=0;
                        bcp_que.push(all_lines_current_gate[i].first);
                        return true;
                    }
                }
            }

        }
        else if(output_line_status==-1&&input_line_status[0]>0&&input_line_status[1]>0)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(output_line_status==-1&&(input_line_status[0]==2||input_line_status[1]==2))
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else
        {
            return true;
        }
        
    }
    case Gate::Type::Not:
    {
        if((input_line_status[0]==1&&output_line_status==0)
        ||(input_line_status[1]==1&&output_line_status==1))
        {
            return false;
        }
        else if(input_line_status[0]==1&&output_line_status==-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(input_line_status[1]==1&&output_line_status==-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(input_line_status[2]==1)
        {
            lines_status_num.at(all_lines_current_gate[0].first)=1-all_lines_current_gate[1].second;
            bcp_que.push(all_lines_current_gate[0].first);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Buff:
    {
         if((input_line_status[0]==1&&output_line_status==1)
         ||(input_line_status[1]==1&&output_line_status==0))
        {
            return false;
        }
        else if(input_line_status[0]==1&&output_line_status==-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=0;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(input_line_status[1]==1&&output_line_status==-1)
        {
            lines_status_num.at(all_lines_current_gate[number_lineOfGate-1].first)=1;
            bcp_que.push(all_lines_current_gate[number_lineOfGate-1].first);
            return true;
        }
        else if(input_line_status[2]==1)
        {
            lines_status_num.at(all_lines_current_gate[0].first)=all_lines_current_gate[1].second;
            bcp_que.push(all_lines_current_gate[0].first);
            return true;
        }
        else
        {
            return true;
        }
    }
    case Gate::Type::Undefined:
    {
        
        return true;
    }
    default:
        return true;
    }

}
}