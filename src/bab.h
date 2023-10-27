#pragma once
#include <vector>
#include <algorithm>
#include <limits>
#include "qap.h"

namespace qap
{
    struct BranchAndBound
    {

        struct Solution
        {
            std::vector<int> permutation;
            int cost;
        };

        static inline int calculate_cost(QAP &problem, std::vector<int> &permutation)
        {
            int cost = 0;
            for (int i = 0; i < problem.n; ++i)
            {
                for (int j = 0; j < problem.n; ++j)
                {
                    cost += problem.distance[i][j] * problem.flow[i][j];
                }
            }
            return cost;
        }

        static inline void branch_and_bound(QAP &problem, std::vector<int> &best_permutation, int &best_cost, std::vector<int> &current_permutation, int current_cost, int level)
        {
            if (level == problem.n)
            {
                if (current_cost < best_cost)
                {
                    best_cost = current_cost;
                    best_permutation = current_permutation;
                }
                return;
            }

            for (int i = level; i < problem.n; ++i)
            {
                std::swap(current_permutation[i], current_permutation[level]);
                int new_cost = calculate_cost(problem,current_permutation);
                if (new_cost < best_cost)
                {
                    branch_and_bound(problem, best_permutation, best_cost, current_permutation, new_cost, level + 1);
                }
                std::swap(current_permutation[i], current_permutation[level]);
            }
        }

        static inline Solution solve(QAP &problem)
        {
            std::vector<int> permutation(problem.n);
            for (int i = 0; i < problem.n; ++i)
            {
                permutation[i] = i;
            }

            std::vector<int> best_permutation(problem.n);
            int best_cost = std::numeric_limits<int>::max();

            branch_and_bound(problem, best_permutation, best_cost, permutation, 0, 0);

            Solution solution{
                .permutation = best_permutation,
                .cost = best_cost
            };

            return solution;
        }

    }; 

} // namepsace qap