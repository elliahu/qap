#pragma once
#include <vector>
#include <algorithm>
#include <limits>
#include <mutex>
#include "qap.h"
#include "thread_pool.h"

namespace qap
{
    /// @brief Struct used to encapsulate logic of solving a singular instance of QAP using BAB algorithm
    struct BranchAndBound
    {
        threading::ThreadPool thread_pool{}; // threadpool used to queue jobs
        std::mutex best_cost_mutex;
        std::mutex best_permutation_mutex;

        /// @brief Solution of the QAP
        struct Solution
        {
            Permutation permutation; // best permutation
            int cost;                // cost of the best permutation
        };

        /// @brief Function to initialize the permutation deterministically
        /// @param perm Permutations
        inline void initialize_permutation(Permutation &perm)
        {
            for (int i = 0; i < perm.size(); ++i)
            {
                perm[i] = i;
            }
        }

        /// @brief Function to set initial permutation consistently
        /// @param n size of the permutations
        /// @return generated permutation
        inline Permutation get_initial_permutation(int n)
        {
            Permutation initial_permutation(n);
            initialize_permutation(initial_permutation);
            return initial_permutation;
        }

        /// @brief Calculates the cost of a permutation
        /// @param problem instance of QAP problem
        /// @param permutation permutation for which the cost will be calculated
        /// @return returns the cost of the permutation
        inline int calculate_cost(QAP &problem, Permutation &permutation)
        {
            int cost = 0;
            for (int i = 0; i < problem.n; i++)
            {
                for (int j = 0; j < problem.n; j++)
                {
                    cost += problem.distance[i][j] * problem.flow[permutation[i]][permutation[j]];
                }
            }
            return cost;
        }

        /// @brief Calculates lower bound at current permutation
        /// @param problem QAP problem
        /// @param current_permutation current permutation
        /// @param level level to which the permutation is set
        /// @return returns calculated lower bound
        inline int calculate_lower_bound(QAP &problem, Permutation &current_permutation, int level)
        {
            int lower_bound = 0;
            for (int i = 0; i < std::min(problem.n, level); i++)
            {
                for (int j = 0; j < std::min(problem.n, level); j++)
                {
                    lower_bound += problem.distance[i][j] * problem.flow[current_permutation[i]][current_permutation[j]];
                }
            }
            return lower_bound;
        }

        /// @brief Recursive function for solving QAP problem using BAB algorithm
        /// @param problem instance of QAP problem
        /// @param best_permutation currently best permutation, top-most function on callstack sets this to best permutation over-all
        /// @param best_cost currently best cost, top-most function on callstack sets this to best cost over-all
        /// @param current_permutation permutation being evaluated at the current call
        /// @param level level of deepness, starts at 0
        inline void branch_and_bound(
            QAP &problem,
            Permutation &best_permutation,
            int &best_cost,
            Permutation &current_permutation,
            int level)
        {
            if (level == problem.n)
            {
                best_cost_mutex.lock();
                best_permutation_mutex.lock();
                best_cost = calculate_cost(problem, current_permutation);
                best_permutation = current_permutation;
                best_cost_mutex.unlock();
                best_permutation_mutex.unlock();
            }
            else
            {
                int lower_bound = calculate_lower_bound(problem, current_permutation, level);

                best_cost_mutex.lock();
                bool prune = lower_bound >= best_cost;
                best_cost_mutex.unlock();

                if (!prune)
                {
                    for (int i = level; i < problem.n; i++)
                    {
                        std::swap(current_permutation[i], current_permutation[level]);
                        int new_cost = calculate_cost(problem, current_permutation);

                        best_cost_mutex.lock();
                        bool should_branch = new_cost < best_cost;
                        best_cost_mutex.unlock();

                        if (should_branch)
                        {
                            branch_and_bound(
                                problem,
                                best_permutation,
                                best_cost,
                                current_permutation,
                                level + 1);
                        }

                        std::swap(current_permutation[i], current_permutation[level]);
                    }
                }
                else
                {
                    return;
                }
            }
        }

        /// @brief Threaded function for solving QAP problem using BAB algorithm
        /// @param problem instance of QAP problem
        /// @param best_permutation currently best permutation, top-most function on callstack sets this to best permutation over-all
        /// @param best_cost currently best cost, top-most function on callstack sets this to best cost over-all
        /// @param level level of deepness, starts at 0
        inline void branch_and_bound_threaded(
            QAP &problem,
            Permutation &best_permutation,
            int &best_cost,
            std::array<Permutation, 12> &initial_permutations)
        {
            for (int i = 0; i < problem.n; i++)
            {
                thread_pool.queue_job(
                    [&, i]
                    {
                        branch_and_bound(problem, best_permutation, best_cost, initial_permutations[i], 1);
                    });
            }
        }

        /// @brief Helper function that starts the BAB computation and returns the best solution found
        /// @param problem instance of QAP problem
        /// @return solution of the QAP problem
        inline Solution solve(QAP &problem)
        {
            std::array<Permutation, 12> initial_permutations{};
            for (int i = 0; i < 12; ++i)
            {
                Permutation current_permutation = get_initial_permutation(problem.n);
                for (int j = 0; j < i; j++)
                    std::swap(current_permutation[j], current_permutation[0]);
                initial_permutations[i] = current_permutation;
            }

            Permutation best_permutation = initial_permutations[0];
            int best_cost = std::numeric_limits<int>::max();

            thread_pool.start();
            branch_and_bound_threaded(problem, best_permutation, best_cost, initial_permutations);

            while (thread_pool.busy())
                continue;

            thread_pool.stop();

            return {
                .permutation = best_permutation,
                .cost = best_cost};
        }
    };

} // namepsace qap