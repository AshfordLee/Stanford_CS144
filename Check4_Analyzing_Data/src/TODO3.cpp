#include<iostream>
#include<fstream>
#include<string>
#include<regex>
#include<vector>

int main()
{
    std::ifstream file("../data/baidu.txt");

    if(!file.is_open())
    {
        std::cerr<<"File cannot be opened!"<<std::endl;
        return 1;
    }

    std::string line;
    std::regex icmp_seq_regex(R"(icmp_seq=(\d+))");
    std::smatch match;
    std::vector<int> icmp_seq_vec;

    while(std::getline(file,line))
    {
        if(std::regex_search(line,match,icmp_seq_regex))
        {
            int icmp_seq=std::stoi(match[1]);
            icmp_seq_vec.push_back(icmp_seq);
        }
    }

    int max_count=0;

    for(auto it=icmp_seq_vec.begin();it!=icmp_seq_vec.end()-1;it++)
    {
        if((*(it+1)-*it)!=1)
        {
            max_count=std::max(max_count,*(it+1)-*it);
        }
    }

    std::cout<<"The max loss is "<<max_count<<std::endl;
}