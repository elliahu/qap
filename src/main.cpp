#include <iostream>
#include "parser.h"

void print_matrix(qap::Mat& mat)
{
    for(auto& i: mat)
    {
        for(auto& j: i)
        {
            std::cout << "\t"<< j;
        }
        std::cout  << std::endl;
    }
    std::cout  << std::endl;
}

int main()
{
    qap::Parser parser{};
    parser.parse("data/data.txt");

    qap::Mat D ,F;

    parser.get_D(D);
    parser.get_F(F);

    print_matrix(D);

    print_matrix(F);


    return 0;
}