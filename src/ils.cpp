#include <chrono>
#include <future>
#include <thread>

#include "ils.h"
#include "problem.h"

using namespace ILS;

static int32_t scanNeighborhood(Problem::Solution &Solution, long int &Budget);
// static bool canSwap(const Problem::Instance &, const std::vector<size_t> &,
//                    size_t I, size_t J, float);

Problem::Solution ILS::solveInstance(const Problem::Instance &Instance,
                                     Config Config) {
    std::default_random_engine RandomGenerator;
    RandomGenerator.seed(Config.RandomSeed);
    long int Budget = Config.Evaluations;

    auto NumIt                   = 0;
    std::vector<size_t> Schedule = Problem::constructSchedule(Instance);
    Problem::Solution CurrentSolution(Instance, Schedule);
    applyLocalSearch(CurrentSolution, Budget);

    auto CurrentMakespan = CurrentSolution.GetMakespan();

    while (Budget > 0) {
        auto CandidateSolution = CurrentSolution;
        applyPerturbation(CandidateSolution, Config.PerturbationStrength,
                          RandomGenerator);
        applyLocalSearch(CandidateSolution, Budget);
        // Evaluates the schedule with perturbation
        auto CandidateMakespan = CandidateSolution.GetMakespan();

        if (CurrentMakespan > CandidateMakespan) {
            CurrentSolution = CandidateSolution;
            CurrentMakespan = CandidateMakespan;
        }
        ++NumIt;
    }

    assert(CurrentSolution.IsFeasible());
    // std::cout << NumIt << " iterations run\n";
    return CurrentSolution;
}

void ILS::applyPerturbation(Problem::Solution &Solution,
                            float PerturbationStrength,
                            std::default_random_engine &RandomGenerator) {
    auto Size       = Solution.Size();
    auto NumOfSwaps = Size * PerturbationStrength / 2;
    std::uniform_int_distribution<size_t> Distribution(0, Size - 1);

    while (NumOfSwaps-- > 0) {
        auto I = Distribution(RandomGenerator);
        auto J = Distribution(RandomGenerator);
        Solution.SwapTasks(std::min(I, J), std::max(I, J));
    }
}

static int32_t scanNeighborhood(Problem::Solution &Solution, long int &Budget) {
    auto Size            = Solution.Size();
    auto CurrentMakespan = Solution.GetMakespan();

    for (size_t I = 0; I < Size - 1; ++I) {
        for (size_t J = I + 1; J < Size; ++J) {
            if (Budget <= 0)
                return -1;

            Solution.SwapTasks(I, J);
            --Budget;
            if (Solution.GetMakespan() < CurrentMakespan) {
                return Solution.GetMakespan();
            } else
                // Not a better solution. Undo the swap
                Solution.SwapTasks(I, J);
        }
    }
    return -1;
}

void ILS::applyLocalSearch(Problem::Solution &Solution, long int &Budget) {
    while (Budget > 0 && scanNeighborhood(Solution, Budget) > 0) {
        continue;
    }
}

/*
static bool canSwap(const Problem::Instance &Instance,
                    const std::vector<size_t> &Schedule, size_t I, size_t J,
                    float RThreshold) {
    assert(I <= J && "Range [I, J] is invalid!");
    assert(Schedule.size() > 0 && "Schedule is empty!");

    auto &Nodes       = Instance.Nodes;
    auto GreatestRisk = Nodes[Schedule[I]].Risk;
    auto SmallestRisk = Nodes[Schedule[J]].Risk;

    // Gets the greatest and smallest risk in the range [I, J]
    for (auto K = I + 1; K < J; ++K) {
        if (Nodes[Schedule[K]].Risk > GreatestRisk)
            GreatestRisk = Nodes[Schedule[K]].Risk;
        else if (Nodes[Schedule[K]].Risk < SmallestRisk)
            SmallestRisk = Nodes[Schedule[K]].Risk;
    }

    return (Problem::canRelaxPriority(Nodes[Schedule[I]].Risk, SmallestRisk,
                                      RThreshold) &&
            Problem::canRelaxPriority(GreatestRisk, Nodes[Schedule[J]].Risk,
                                      RThreshold));
}
*/