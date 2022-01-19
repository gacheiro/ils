#include <string>

#include "ils.h"
#include "problem.h"

std::string InstanceFile;
uint32_t TimeLimit         = 2;
float PerturbationStrength = 0.5;
float RelaxationThreshold  = 0;
int RandomSeed             = 0;

int parseCommandLine(int Argc, char *Argv[]) {
    const auto HELP_MSG =
        " Iterated Local Search implementation for the [untitled] problem\n\n"
        " USAGE:\n\n  ./ils [OPTIONS] INSTANCE_PATH\n\n"
        " OPTIONS:\n\n"
        " -h, --help\n"
        " \tShow this message and exit\n\n"
        " -p [THRESHOLD]\n"
        " \tThreshold for relxation of priority rules, in range [0, 1]\n\n"
        " --time-limit [LIMIT]\n"
        " \tThe maximum time to execute the program (in seconds)\n\n"
        " --pstrength [STRENGTH]\n"
        " \tThe pertubation strength factor (in range [0, 1])\n\n"
        " --seed [SEED]\n"
        " \tThe seed for the random number generator\n\n";

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

        else if ((Arg == "--seed"))
            if (I + 1 < Argc)
                RandomSeed = std::stoi(Argv[++I]);
            else {
                std::cout << "--seed option requires one argument\n";
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

    std::cout << "\nRunning instance " << InstanceFile << " with parameters"
              << " --time-limit " << TimeLimit << " --pstrength "
              << PerturbationStrength << " -p " << RelaxationThreshold
              << " --seed " << RandomSeed << "\n";

    Problem::Instance Instance =
        Problem::loadInstance(InstanceFile, RelaxationThreshold);

    // Defines the seed for the random number generator
    ILS::RandomGenerator.seed(RandomSeed);
    // Defines the threshold for relaxation of priority rules
    ILS::RELAXATION_THRESHOLD = RelaxationThreshold;
    Problem::Solution Solution =
        ILS::solveInstance(Instance, PerturbationStrength, TimeLimit);
    Solution.PrintSchedule();
    return 0;
}
