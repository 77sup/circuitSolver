#include "../include/solver.h"

solver::solver(const CircuitGraph& graph)
{
    std::vector<std::string> noPI_lines_name;
    std::vector<std::string> output; 
    std::cout<<graph.get_name_to_line().size()<<std::endl;
    for(int i=0;i<graph.get_lines().size();i++)
    {
        if(graph.get_lines()[i].is_output) output.push_back(graph.get_lines()[i].name);
        else noPI_lines_name.push_back(graph.get_lines()[i].name);
    }
    for(int i=0;i<noPI_lines_name.size();i++){
        lines_status.insert(make_pair(noPI_lines_name[i],-1));
        if(!graph.get_name_to_line().at(noPI_lines_name[i])->source){
            the_name_of_input_line.push_back(noPI_lines_name[i]);
        }
    }
    //print the_name_of_input_line
    for(int i=0;i<the_name_of_input_line.size();i++){
        std::cout<<"the_name_of_input_line:"<<the_name_of_input_line[i]<<std::endl;
    }
    for(int i=0;i<output.size();i++){
        lines_status.insert(make_pair(output[i],1));
    }
    //test
    std::cout<<std::endl;
    for(auto it=lines_status.begin();it!=lines_status.end();it++){
        std::cout<<"the_status_of_line:"<<(*it).first<<"    "<<(*it).second<<std::endl;
    }

    std::string change;
    for(int i=0;i<noPI_lines_name.size()-1;i++){
        for(int j=i+1;j<noPI_lines_name.size();j++){
            if(graph.get_name_to_line().at(noPI_lines_name[i])->destination_gates.size()<graph.get_name_to_line().at(noPI_lines_name[j])->destination_gates.size()){
                change=noPI_lines_name[i];
                noPI_lines_name[i]=noPI_lines_name[j];
                noPI_lines_name[j]=change;
            }
        }
    }
    for(int i=0;i<noPI_lines_name.size();i++){
        sort_destination_gates.push_back(noPI_lines_name[i]);
    }
    //test
    for(int i=0;i<sort_destination_gates.size();i++){
        std::cout<<"sort_line_name:"<<sort_destination_gates[i]<<" num:  "<<graph.get_name_to_line().at(sort_destination_gates[i])->destination_gates.size()<<std::endl;
    }
}
std::string solver::FindDecisionTarget(std::unordered_map<std::string,int>& lines_status)
{
    std::string Target;
    for(int i=0; i<sort_destination_gates.size();i++){
        if(lines_status[sort_destination_gates[i]]==-1){
            Target = sort_destination_gates[i];
            break;
        }
    }
    return Target;
}
bool solver::SingleGateReasoning(Gate*,std::string)
{
    return false;
}