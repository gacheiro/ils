#include <chrono>
#include <future>
#include <string>
#include <ctime>
#include <thread>

#include "ils.h"
#include "problem.h"

std::string InstanceFile;
Problem::Model Model      = Problem::Model::MinMakespan;
uint32_t TimeLimit        = 10;
float PertubationStrength = 0.5;

int parseCommandLine(int Argc, char *Argv[]) {
    const auto HELP_MSG =
        "  Iterated Local Search implementation\n\n"
        "  USAGE:\n\n  ./ils [OPTIONS] INSTANCE_FILE\n\n"
        "  OPTIONS:\n\n"
        "  -h, --help\n"
        "      Show this message and exit\n\n"
        "  --model [MODEL]\n"
        "      Choose the model to solve the instance. Valid options are\n"
        "      1 for minimizing the makespan (default) or 2 for minimizing\n"
        "      the sum of weighted completion times\n\n"
        "  --time-limit [LIMIT]\n"
        "      The maximum time to execute the program (in seconds)\n\n"
        "  --pstrength [STRENGTH]\n"
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

        else if ((Arg == "--time-limit"))
            if (I + 1 < Argc)
                TimeLimit = std::stoi(Argv[++I]);
            else {
                std::cout << "--time-limit option requires one argument\n";
                return -1;
            }

        else if ((Arg == "--pstrength"))
            if (I + 1 < Argc)
                PertubationStrength = std::stof(Argv[++I]);
            else {
                std::cout << "--pstrength option requires one argument\n";
                return -1;
            }

        else if ((Arg == "--model"))
            if (I + 1 < Argc) {
                Model = std::stoi(Argv[++I]) == 1
                            ? Problem::Model::MinMakespan
                            : Problem::Model::MinWeightedCompletionTime;
            } else {
                std::cout << "--model option requires one argument\n";
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
              << " --model " << Model + 1 << " --time-limit " << TimeLimit
              << " --pstrength " << PertubationStrength << "\n";

    Problem::Instance Instance = Problem::loadInstance(InstanceFile);

    // We start the optimization in another thread while this one
    // is responsible for accounting the time limit and sinalyzing
    // when the time limit is reached
    std::future<Problem::Solution> FutureSolution =
        std::async(ILS::solveInstance, Model, Instance, PertubationStrength);

    std::this_thread::sleep_for(std::chrono::seconds(TimeLimit));
    ILS::TIME_LIMIT_EXCEEDED = true;

    std::cout << "Objective = " << FutureSolution.get().Objective << std::endl;
    return 0;
}
