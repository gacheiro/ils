#include <string>

#include "ils.h"
#include "problem.h"

std::string InstancePath;
long int Evaluations       = -1;
float PerturbationStrength = 0.5;
float RelaxationThreshold  = 0;
int RandomSeed             = 0;
bool SetupTimes            = true;

int parseCommandLine(int Argc, char *Argv[]) {
    const auto HELP_MSG =
        " Iterated Local Search implementation for the [untitled] problem\n\n"
        " USAGE:\n\n  ./ils [OPTIONS] INSTANCE_PATH\n\n"
        " OPTIONS:\n\n"
        " -h, --help\n"
        " \tShow this message and exit\n\n"
        " --evaluations [BUDGET]\n"
        " \tNumber of calls to evaluation function.\n"
        " \tdefault is -1 (sets automatically)\n"
        " --no-setup-times\n"
        " \tDisable setup times (optimise only scheduling problem)\n\n"
        " --perturbation [STRENGTH]\n"
        " \tPertubation strength factor (in range [0, 1])\n\n"
        " --relaxation [THRESHOLD]\n"
        " \tThreshold for relaxation of priority rules, in range [0, 1]\n\n"
        " --seed [SEED]\n"
        " \tSeed for random number generator\n\n";

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

        else if ((Arg == "--relaxation")) {
            if (I + 1 < Argc)
                RelaxationThreshold = std::stof(Argv[++I]);
            else {
                std::cout << "--relaxation option requires one argument\n";
                return -1;
            }
        }

        else if ((Arg == "--evaluations"))
            if (I + 1 < Argc) {
                Evaluations = std::stoi(Argv[++I]);
            }
            else {
                std::cout << "--evaluations option requires one argument\n";
                return -1;
            }

        else if ((Arg == "--perturbation"))
            if (I + 1 < Argc)
                PerturbationStrength = std::stof(Argv[++I]);
            else {
                std::cout << "--perturbation option requires one argument\n";
                return -1;
            }

        else if ((Arg == "--seed"))
            if (I + 1 < Argc)
                RandomSeed = std::stoi(Argv[++I]);
            else {
                std::cout << "--seed option requires one argument\n";
                return -1;
            }

        else if ((Arg == "--no-setup-times"))
            SetupTimes = false;

        // TODO: add a --silent mode

        else
            InstancePath = Arg;
    }

    return 0;
}

int main(int Argc, char *Argv[]) {

    if (parseCommandLine(Argc, Argv) == -1)
        return -1;

    // std::cout << "\nRunning instance " << InstanceFile << " with parameters"
    //          << " --budget " << MaxBudget << " --pstrength "
    //          << PerturbationStrength << " -p " << RelaxationThreshold
    //          << " --seed " << RandomSeed << "\n";

    Problem::Config ProblemConfig = {RelaxationThreshold, SetupTimes};
    Problem::Instance Instance =
        Problem::loadInstance(InstancePath, ProblemConfig);

    // Automatically calculates the amount of evaluation calls
    // based on the number of nodes
    if (Evaluations <= 0)
        Evaluations = 10000 * Instance.NumOfNodes;

    ILS::Config ILSConfig = {RelaxationThreshold, PerturbationStrength,
                             Evaluations, RandomSeed};

    Problem::Solution Solution = ILS::solveInstance(Instance, ILSConfig);

    //  Solution.PrintSchedule();
    std::cout << Solution.GetMakespan() << '\n';
    return 0;
}
