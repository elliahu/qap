#include <iostream>
#include "parser.h"
#include "qap.h"
#include "bab.h"

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

    qap::QAP qap{
        .n = parser.size,
        .distance = parser.get_D(),
        .flow = parser.get_F()
    };

    int min_cost = solveQAP(qap);
    std::cout << "Minimum cost: " << min_cost << std::endl;

    return 0;
}