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
        std::mutex current_permutation_mutex;

        /// @brief Solution of the QAP
        struct Solution
        {
            Permutation permutation; // best permutation
            int cost;                // cost of the best permutation
        };

        /// @brief Function to initialize the permutation deterministically
        /// @param perm Permutations
        void initialize_permutation(Permutation &perm)
        {
            for (int i = 0; i < perm.size(); ++i)
            {
                perm[i] = i;
            }
        }

        /// @brief Function to set initial permutation consistently
        /// @param n size of the permutations
        /// @return generated permutation
        Permutation get_initial_permutation(int n)
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
            for (int i = 0; i < problem.n; ++i)
            {
                for (int j = 0; j < problem.n; ++j)
                {
                    cost += problem.distance[i][j] * problem.flow[permutation[i]][permutation[j]];
                }
            }
            return cost;
        }

        /// @brief Function to compute the reduced cost matrix for the current state
        /// @param problem QAP problem
        /// @param current_permutation current permutation
        /// @param reduced_cost reduced cost
        void compute_reduced_cost_matrix(QAP &problem, Permutation &current_permutation, std::vector<std::vector<int>> &reduced_cost)
        {
            int n = problem.n;
            reduced_cost.resize(n, std::vector<int>(n, 0));

            for (int facility1 = 0; facility1 < n; ++facility1)
            {
                for (int facility2 = 0; facility2 < n; ++facility2)
                {
                    int diff = problem.distance[facility1][facility1] - problem.distance[facility1][facility2] +
                               problem.distance[facility2][facility2] - problem.distance[facility2][facility1];

                    reduced_cost[facility1][facility2] = diff * problem.flow[current_permutation[facility1]][current_permutation[facility2]];
                }
            }
        }

        /// @brief Calculates lower bound at current permutation
        /// @param problem QAP problem
        /// @param current_permutation current permutation
        /// @return returns calculated lower bound
        int calculate_lower_bound(QAP &problem, Permutation &current_permutation)
        {
            int lower_bound = 0;
            std::vector<std::vector<int>> reduced_cost;
            compute_reduced_cost_matrix(problem, current_permutation, reduced_cost);

            for (int facility1 = 0; facility1 < problem.n; ++facility1)
            {
                for (int facility2 = 0; facility2 < problem.n; ++facility2)
                {
                    lower_bound += reduced_cost[facility1][facility2];
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
                int current_cost = calculate_cost(problem, current_permutation);
                if (current_cost < best_cost)
                {
                    best_cost = current_cost;
                    best_permutation = current_permutation;
                }
                best_cost_mutex.unlock();
                best_permutation_mutex.unlock();
            }
            else
            {
                int lower_bound = calculate_lower_bound(problem, current_permutation);

                best_cost_mutex.lock();
                bool should_loop = lower_bound < best_cost;
                best_cost_mutex.unlock();

                if (should_loop)
                {
                    for (int i = level; i < problem.n; ++i)
                    {
                        std::swap(current_permutation[i], current_permutation[level]);
                        int new_cost = calculate_cost(problem, current_permutation);

                        best_cost_mutex.lock();
                        bool should_branch = new_cost < best_cost;
                        best_cost_mutex.unlock();

                        if (should_branch)
                        {
                            branch_and_bound(problem, best_permutation, best_cost, current_permutation, level + 1);
                        }

                        std::swap(current_permutation[i], current_permutation[level]);
                    }
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
            Permutation &initial_permutation,
            int level)
        {
            int lower_bound = calculate_lower_bound(problem, initial_permutation);

            if (lower_bound < best_cost)
            {
                for (int i = level; i < problem.n; ++i)
                {
                    thread_pool.queueJob(
                        [&, i, level]
                        {
                            Permutation current_permutation = initial_permutation;
                            std::swap(current_permutation[i], current_permutation[level]);
                            int new_cost = calculate_cost(problem, current_permutation);

                            best_cost_mutex.lock();
                            bool shoul_branch = new_cost < best_cost;
                            best_cost_mutex.unlock();

                            if (shoul_branch)
                            {
                                branch_and_bound(problem, best_permutation, best_cost, current_permutation, level + 1);
                            }

                            std::swap(current_permutation[i], current_permutation[level]);
                        });
                }
            }
        }

        /// @brief Helper function that starts the BAB computation and returns the best solution found
        /// @param problem instance of QAP problem
        /// @return solution of the QAP problem
        inline Solution solve(QAP &problem)
        {
            Permutation initial_permutation = get_initial_permutation(problem.n);
            Permutation best_permutation = initial_permutation;
            int best_cost = std::numeric_limits<int>::max();

            thread_pool.start();

            branch_and_bound_threaded(problem, best_permutation,best_cost, initial_permutation, 0);

            while (thread_pool.busy())
                continue;

            thread_pool.stop();

            return {
                .permutation = best_permutation,
                .cost = best_cost};
        }
    };

} // namepsace qap