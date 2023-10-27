#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "defines.h"

namespace qap
{
    struct Parser
    {
        inline void parse(std::string path, Mat& D, Mat& F, int& size)
        {
            std::ifstream input_file(path);
            if(input_file.fail())
            {
                std::cout << "FILE ERROR" << std::endl;
                exit(EXIT_FAILURE);
            }
            
            std::string line;

            // read the size
            std::getline(input_file, line);
            sscanf(line.c_str(), "%d", &size);

            D.clear();
            D.resize(size);
            for(auto& d: D)
                d.resize(size);

            F.clear();
            F.resize(size);
            for(auto& f: F)
                f.resize(size);

            // skip line
            std::getline(input_file, line);

            // read D matrix
            for (int row = 0; row < size; row++)
            {
                std::getline(input_file, line);
                const char *data = line.c_str();
                int n;
                int offset;
                int col = 0;
                while (sscanf(data, "%d%n", &n, &offset) == 1)
                {
                    data += offset;
                    D[row][col] = n;
                    col++;
                }
            }

            // skip line
            std::getline(input_file, line);

            // read F matrix
            for (int row = 0; row < size; row++)
            {
                std::getline(input_file, line);
                const char *data = line.c_str();
                int n;
                int offset;
                int col = 0;
                while (sscanf(data, "%d%n", &n, &offset) == 1)
                {
                    data += offset;
                    F[row][col] = n;
                    col++;
                }
            }
        }
    };
}