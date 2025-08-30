#include<iostream>
#include<fstream>
#include<string>

int main()
{
    std::ifstream file("../data/baidu.txt");

    if(!file.is_open())
    {
        std::cerr<<"File cannot be opened!"<<std::endl;
        return 1;
    }

    int count=0;
    std::string line;
    while(std::getline(file,line))
    {
        count++;
    }

    float percent=(count/18000.0f)*100;
    std::cout<<percent<<"% of the pings received"<<std::endl;
    file.close();
    return 0;
}