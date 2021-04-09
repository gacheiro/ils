#include <iostream>
#include <string>

#include "ils.h"
#include "problem.h"

std::string InstanceFile;
uint32_t NumberOfIterations = 10;
float PertubationStrength = 0.5;

int parseCommandLine(int Argc, char *Argv[]) {
    const auto HELP_MSG =
        "  Iterated Local Search implementation\n\n"
        "  USAGE:\n\n  ./ils [OPTIONS] INSTANCE_FILE\n\n"
        "  OPTIONS:\n\n"
        "  -h, --help\n"
        "      Show this message and exit\n\n"
        "  --niterations[=NUMBER]\n"
        "      The number of iterations to run\n\n"
        "  --pstrength[=STRENGTH]\n"
        "      The pertubation strength factor in the interval [0, 1]\n\n";

    if (Argc < 2) {
        std::cout << HELP_MSG;
        return -1;
    }

    for (auto I = 0; I < Argc; ++I) {
        std::string Arg = Argv[I];
        if ((Arg == "-h") || (Arg == "--help")) {
            std::cout << HELP_MSG;
            return -1;
        }

        else if ((Arg == "--niterations"))
            if (I + 1 < Argc) {
                NumberOfIterations = std::stoi(Argv[++I]);
            } else {
                std::cout << "--niterations option requires one argument\n";
                return -1;
            }

        else if ((Arg == "--pstrength"))
            if (I + 1 < Argc) {
                PertubationStrength = std::atof(Argv[++I]);
            } else {
                std::cout << "--pstrength option requires one argument\n";
                return -1;
            }

        else
            InstanceFile = Arg;
    }

    return 0;
}

int main(int Argc, char *Argv[]) {

    if (parseCommandLine(Argc, Argv) == -1) {
        return -1;
    }

    Problem::Instance Instance = Problem::loadInstance(InstanceFile);

    // Initializes the global distance matrix
    Problem::DistMatrix =
        Problem::GetDistanceMatrix(Instance.Nodes, Instance.Edges);
    std::cout << NumberOfIterations << " " << PertubationStrength << "\n";
    Problem::Solution Solution =
        ILS::solveInstance(Instance, PertubationStrength, NumberOfIterations);
    std::cout << "Objective = " << Solution.Objective << std::endl;
    return 0;
}
