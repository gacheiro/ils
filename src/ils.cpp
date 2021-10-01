#include <chrono>
#include <ctime>
#include <future>
#include <random>
#include <thread>

#include "ils.h"
#include "problem.h"

using namespace ILS;

float ILS::RELAXATION_THRESHOLD = 0;

static std::default_random_engine RandomGenerator;

static int32_t scanNeighborhood(const Problem::Instance &,
                                std::vector<size_t> &);
static bool canSwap(const Problem::Instance &, const std::vector<size_t> &,
                    size_t I, size_t J, float);

static Problem::Solution __solveInstance(const Problem::Instance &Instance,
                                         float PerturbationStrength,
                                         const bool *TLE) {

    // Sort vector by risk in descending order
    std::vector<size_t> CurrentSchedule = Instance.GetDestinationsIds();
    Problem::repairSchedule(Instance, CurrentSchedule);

    applyLocalSearch(Instance, CurrentSchedule, TLE);
    auto CurrentObjective =
        Problem::evaluateSchedule(Instance, CurrentSchedule);

    do {
        // Copies the schedule and evaluates the perturbated schedule
        auto CandidateSchedule = CurrentSchedule;
        applyPerturbation(Instance, CandidateSchedule, PerturbationStrength);

        applyLocalSearch(Instance, CandidateSchedule, TLE);

        // Evaluates the schedule with perturbation
        auto CandidateObjective =
            Problem::evaluateSchedule(Instance, CandidateSchedule);

        if (CurrentObjective > CandidateObjective) {
            CurrentObjective = CandidateObjective;
            CurrentSchedule  = CandidateSchedule;
        }
    } while (!(*TLE));

    Problem::AssertPiorityRules(Instance, CurrentSchedule,
                                ILS::RELAXATION_THRESHOLD);
    return Problem::Solution(CurrentObjective, CurrentSchedule);
}

Problem::Solution ILS::solveInstance(const Problem::Instance &Instance,
                                     float PerturbationStrength,
                                     uint32_t TimeLimit) {
    bool TLE = false;
    // We start the optimization in another thread while this one
    // is responsible for accounting the time limit and signal
    // when the time limit is reached
    std::future<Problem::Solution> FutureSolution = std::async(
        __solveInstance, Instance, PerturbationStrength, &TLE);
    std::this_thread::sleep_for(std::chrono::seconds(TimeLimit));

    TLE = true;
    return FutureSolution.get();
}

void ILS::applyPerturbation(const Problem::Instance &Instance,
                            std::vector<size_t> &Schedule,
                            float PerturbationStrength) {
    auto Size       = Schedule.size();
    auto NumOfSwaps = Size * PerturbationStrength / 2;
    std::uniform_int_distribution<size_t> Distribution(0, Size - 1);

    while (NumOfSwaps-- > 0) {
        auto I = Distribution(RandomGenerator);
        auto J = Distribution(RandomGenerator);

        if (!canSwap(Instance, Schedule, std::min(I, J), std::max(I, J),
                     ILS::RELAXATION_THRESHOLD))
            continue;

        auto Aux    = Schedule[I];
        Schedule[I] = Schedule[J];
        Schedule[J] = Aux;
    }
}

static int32_t scanNeighborhood(const Problem::Instance &Instance,
                                std::vector<size_t> &Schedule) {
    const auto Size = Schedule.size();
    auto CurrentObjective =
        Problem::evaluateSchedule(Instance, Schedule);

    for (size_t I = 0; I < Size - 1; ++I) {
        for (size_t J = I + 1; J < Size; ++J) {

            //        if (ILS::TIME_LIMIT_EXCEEDED)
            //            return -1;

            if (!canSwap(Instance, Schedule, I, J, ILS::RELAXATION_THRESHOLD))
                continue;

            auto Aux    = Schedule[I];
            Schedule[I] = Schedule[J];
            Schedule[J] = Aux;

            auto Objective =
                Problem::evaluateSchedule(Instance, Schedule);

            if (Objective < CurrentObjective)
                return Objective;
            else {
                // Not a better solution. Undo the swap
                Schedule[J] = Schedule[I];
                Schedule[I] = Aux;
            }
        }
    }
    return -1;
}

void ILS::applyLocalSearch(const Problem::Instance &Instance,
                           std::vector<size_t> &Schedule, const bool *TLE) {
    while (TLE != NULL && !(*TLE)) {
        if (scanNeighborhood(Instance, Schedule) == -1)
            break;
    }
}

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
