#include <string>

#include "ils.h"
#include "problem.h"

std::string InstanceFile;
uint32_t TimeLimit         = 2;
float PerturbationStrength = 0.5;
float RelaxationThreshold  = 0;

int parseCommandLine(int Argc, char *Argv[]) {
    const auto HELP_MSG =
        "  Iterated Local Search implementation for the [untitled]\n\n"
        "  USAGE:\n\n  ./ils [OPTIONS] INSTANCE_FILE\n\n"
        "  OPTIONS:\n\n"
        "  -h, --help\n"
        "      Show this message and exit\n\n"
        "  -p [THRESHOLD]\n"
        "      Threshold for relxation of priority rules, in range [0, 1]\n\n"
        "  --time-limit [LIMIT]\n"
        "      The maximum time to execute the program (in seconds)\n\n"
        "  --pstrength [STRENGTH]\n"
        "      The pertubation strength factor, in range [0, 1]\n\n";

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

        else if ((Arg == "-p")) {
            if (I + 1 < Argc)
                RelaxationThreshold = std::stof(Argv[++I]);
            else {
                std::cout << "-p option requires one argument\n";
                return -1;
            }
        }

        else if ((Arg == "--time-limit"))
            if (I + 1 < Argc)
                TimeLimit = std::stoi(Argv[++I]);
            else {
                std::cout << "--time-limit option requires one argument\n";
                return -1;
            }

        else if ((Arg == "--pstrength"))
            if (I + 1 < Argc)
                PerturbationStrength = std::stof(Argv[++I]);
            else {
                std::cout << "--pstrength option requires one argument\n";
                return -1;
            }

        else
            InstanceFile = Arg;
    }

    return 0;
}

int main(int Argc, char *Argv[]) {

    if (parseCommandLine(Argc, Argv) == -1)
        return -1;

    // TODO: add a parameter for the random seed
    std::cout << "\nRunning instance " << InstanceFile << " with parameters"
              << " --time-limit " << TimeLimit << " --pstrength "
              << PerturbationStrength << " -p " << RelaxationThreshold << "\n";

    Problem::Instance Instance = Problem::loadInstance(InstanceFile);

    // Defines the threshold for relaxation of priority rules
    ILS::RELAXATION_THRESHOLD = RelaxationThreshold;
    Problem::Solution Solution =
        ILS::solveInstance(Instance, PerturbationStrength, TimeLimit);

    std::cout << "Objective = " << Solution.Objective << std::endl;
    std::cout << "Schedule = ";
    for (auto I : Solution.Schedule) {
        std::cout << Instance.Nodes[I].Risk << " ";
    }
    std::cout << std::endl;

    return 0;
}
