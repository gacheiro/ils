#include <chrono>
#include <future>
#include <thread>

#include "ils.h"
#include "problem.h"

using namespace ILS;

float ILS::RELAXATION_THRESHOLD = 0;
std::default_random_engine ILS::RandomGenerator;

static int32_t scanNeighborhood(Problem::Solution &Solution);
// static bool canSwap(const Problem::Instance &, const std::vector<size_t> &,
//                    size_t I, size_t J, float);

static Problem::Solution __solveInstance(const Problem::Instance &Instance,
                                         float PerturbationStrength,
                                         const bool *TLE) {
    auto NumIt                   = 0;
    std::vector<size_t> Schedule = Problem::constructSchedule(Instance);
    Problem::Solution CurrentSolution(Instance, Schedule);
    applyLocalSearch(CurrentSolution, TLE);

    auto CurrentMakespan = CurrentSolution.GetMakespan();

    do {
        auto CandidateSolution = CurrentSolution;
        applyPerturbation(CandidateSolution, PerturbationStrength);
        applyLocalSearch(CandidateSolution, TLE);
        // Evaluates the schedule with perturbation
        auto CandidateMakespan = CandidateSolution.GetMakespan();

        if (CurrentMakespan > CandidateMakespan) {
            CurrentSolution = CandidateSolution;
            CurrentMakespan = CandidateMakespan;
        }
        ++NumIt;
    } while (!(*TLE));

    assert(CurrentSolution.IsFeasible());
    std::cout << NumIt << " iterations run\n";
    return CurrentSolution;
}

Problem::Solution ILS::solveInstance(const Problem::Instance &Instance,
                                     float PerturbationStrength,
                                     uint32_t TimeLimit) {
    bool TLE = false;
    // We start the optimization in another thread while this one
    // is responsible for accounting the time limit and signaling
    // when the time limit is reached
    std::future<Problem::Solution> FutureSolution =
        std::async(__solveInstance, Instance, PerturbationStrength, &TLE);
    std::this_thread::sleep_for(std::chrono::seconds(TimeLimit));

    TLE = true;
    return FutureSolution.get();
}

void ILS::applyPerturbation(Problem::Solution &Solution,
                            float PerturbationStrength) {
    auto Size       = Solution.Size();
    auto NumOfSwaps = Size * PerturbationStrength / 2;
    std::uniform_int_distribution<size_t> Distribution(0, Size - 1);

    while (NumOfSwaps-- > 0) {
        auto I = Distribution(RandomGenerator);
        auto J = Distribution(RandomGenerator);
        Solution.SwapTasks(std::min(I, J), std::max(I, J));
        /*    if (!canSwap(Instance, Schedule, std::min(I, J), std::max(I, J),
                         ILS::RELAXATION_THRESHOLD))
                continue;

            auto Aux    = Schedule[I];
            Schedule[I] = Schedule[J];
            Schedule[J] = Aux;
        */
    }
}

static int32_t scanNeighborhood(Problem::Solution &Solution) {
    auto Size            = Solution.Size();
    auto CurrentMakespan = Solution.GetMakespan();

    for (size_t I = 0; I < Size - 1; ++I) {
        for (size_t J = I + 1; J < Size; ++J) {
            Solution.SwapTasks(I, J);
            /*          if (!canSwap(Instance, Schedule, I, J,
               ILS::RELAXATION_THRESHOLD)) continue;

                        auto Aux    = Schedule[I];
                        Schedule[I] = Schedule[J];
                        Schedule[J] = Aux;
            */
            if (Solution.GetMakespan() < CurrentMakespan)
                return Solution.GetMakespan();
            else
                // Not a better solution. Undo the swap
                Solution.SwapTasks(I, J);
        }
    }
    return -1;
}

void ILS::applyLocalSearch(Problem::Solution &Solution, const bool *TLE) {
    while (TLE != NULL && !(*TLE)) {
        if (scanNeighborhood(Solution) == -1)
            break;
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