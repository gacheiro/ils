#include <iostream>

#include "ils.h"
#include "problem.h"

using namespace ILS;

bool ILS::TIME_LIMIT_EXCEEDED = false;

static inline void applyPerturbation(std::vector<size_t> &, float);
static int32_t scanNeighborhood(const Problem::Instance &,
                                std::vector<size_t> &);

Problem::Solution ILS::solveInstance(const Problem::Instance &Instance,
                                     float PerturbationStrength) {

    // Sort vector by risk in descending order
    std::vector<size_t> CurrentSchedule = Instance.GetDestinationsIds();
    Problem::repairSchedule(Instance, CurrentSchedule);

    applyLocalSearch(Instance, CurrentSchedule);
    auto CurrentObjective =
        Problem::evaluateSchedule(Instance, CurrentSchedule);

    do {
        // Copies the schedule and evaluates the perturbated schedule
        auto CandidateSchedule = CurrentSchedule;
        applyPerturbation(CandidateSchedule, PerturbationStrength);

        // Sort vector by risk in descending order
        Problem::repairSchedule(Instance, CandidateSchedule);

        applyLocalSearch(Instance, CandidateSchedule);

        // Evaluates the schedule with perturbation
        auto CandidateObjective =
            Problem::evaluateSchedule(Instance, CandidateSchedule);

        if (CurrentObjective > CandidateObjective) {
            CurrentObjective = CandidateObjective;
            CurrentSchedule  = CandidateSchedule;
        }
    } while (!ILS::TIME_LIMIT_EXCEEDED);

    return Problem::Solution(CurrentObjective, CurrentSchedule);
}

static inline void applyPerturbation(std::vector<size_t> &Schedule,
                                     float PerturbationStrength) {
    auto Size       = Schedule.size();
    auto NumOfSwaps = Size * PerturbationStrength / 2;

    do {
        auto I      = rand() % Size;
        auto J      = rand() % Size;
        auto Aux    = Schedule[I];
        Schedule[I] = Schedule[J];
        Schedule[J] = Aux;
    } while (--NumOfSwaps > 0);
}

static int32_t scanNeighborhood(const Problem::Instance &Instance,
                                std::vector<size_t> &Schedule) {
    const auto Size       = Schedule.size();
    auto CurrentObjective = Problem::evaluateSchedule(Instance, Schedule);

    for (size_t I = 0; I < Size - 1; ++I) {
        for (size_t J = I + 1; J < Size; ++J) {

            if (ILS::TIME_LIMIT_EXCEEDED)
                return -1;

            // Performs a swap between jobs at indices I and J
            // with respect to precedence constraints.
            // This way we keep the schedule feasible.
            if (Instance.Nodes[Schedule[I]].Risk >
                Instance.Nodes[Schedule[J]].Risk)
                continue;

            auto Aux    = Schedule[I];
            Schedule[I] = Schedule[J];
            Schedule[J] = Aux;

            auto Objective = Problem::evaluateSchedule(Instance, Schedule);

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
                           std::vector<size_t> &Schedule) {
    for (size_t I = 0; scanNeighborhood(Instance, Schedule) != -1; ++I);
}
