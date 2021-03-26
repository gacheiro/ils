#include <iostream>
#include <string>

#include "ils.h"
#include "problem.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "usage: main path/to/instance.dat\n";
        return -1;
    }
    Problem::Instance Instance = Problem::loadInstance(argv[1]);
    Problem::Solution Solution = ILS::solveInstance(Instance, 0.5, 10);
    std::cout << "Objective = " << Solution.Objective << std::endl;
    return 0;
}
