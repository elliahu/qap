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
        Mat D, F;
        size_t size;

        inline void parse(std::string path)
        {
            std::ifstream input_file(path);
            std::string line;

            // read the size
            std::getline(input_file, line);
            sscanf(line.c_str(), "%d", &size);
            std::cout << "Reading the size of " << size << std::endl;

            D = Mat(size, std::vector<int>(size));
            F = Mat(size, std::vector<int>(size));

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

        inline void get_D(Mat &d)
        {
            d.clear();
            d = Mat(size, std::vector<int>(size));
            for(int i = 0; i < size; i++)
                for(int j = 0; j < size; j++)
                    d[i][j] = D[i][j];
        }

        inline void get_F(Mat &f)
        {
            f.clear();
            f = Mat(size, std::vector<int>(size));
            for(int i = 0; i < size; i++)
                for(int j = 0; j < size; j++)
                    f[i][j] = F[i][j];
        }
    };
}