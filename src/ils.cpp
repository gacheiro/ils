#include <random>

#include "ils.h"
#include "problem.h"

using namespace ILS;

bool ILS::TIME_LIMIT_EXCEEDED = false;

static std::default_random_engine RandomGenerator;

static inline void applyPerturbation(std::vector<size_t> &, float);
static int32_t scanNeighborhood(Problem::Model Model, const Problem::Instance &,
                                std::vector<size_t> &);

Problem::Solution ILS::solveInstance(Problem::Model Model,
                                     const Problem::Instance &Instance,
                                     float PerturbationStrength) {

    // Sort vector by risk in descending order
    std::vector<size_t> CurrentSchedule = Instance.GetDestinationsIds();

    if (Model == Problem::Model::MinMakespan)
        Problem::repairSchedule(Instance, CurrentSchedule);

    applyLocalSearch(Model, Instance, CurrentSchedule);
    auto CurrentObjective =
        Problem::evaluateSchedule(Model, Instance, CurrentSchedule);

    do {
        // Copies the schedule and evaluates the perturbated schedule
        auto CandidateSchedule = CurrentSchedule;
        applyPerturbation(CandidateSchedule, PerturbationStrength);

        if (Model == Problem::Model::MinMakespan)
            Problem::repairSchedule(Instance, CandidateSchedule);

        applyLocalSearch(Model, Instance, CandidateSchedule);

        // Evaluates the schedule with perturbation
        auto CandidateObjective =
            Problem::evaluateSchedule(Model, Instance, CandidateSchedule);

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
    std::uniform_int_distribution<int> Distribution(0, Size - 1);

    while (NumOfSwaps-- > 0) {
        auto I = Distribution(RandomGenerator);
        auto J = Distribution(RandomGenerator);
        auto Aux    = Schedule[I];
        Schedule[I] = Schedule[J];
        Schedule[J] = Aux;
    }
}

static int32_t scanNeighborhood(Problem::Model Model,
                                const Problem::Instance &Instance,
                                std::vector<size_t> &Schedule) {
    const auto Size = Schedule.size();
    auto CurrentObjective =
        Problem::evaluateSchedule(Model, Instance, Schedule);

    for (size_t I = 0; I < Size - 1; ++I) {
        for (size_t J = I + 1; J < Size; ++J) {

            if (ILS::TIME_LIMIT_EXCEEDED)
                return -1;

            // When solving with model MinMakespan, we can only make swaps that
            // respect precedence constraints. In practice, we swap I and J if
            // r[I] <= r[J]
            else if (Model == Problem::Model::MinMakespan &&
                     (Instance.Nodes[Schedule[I]].Risk >
                      Instance.Nodes[Schedule[J]].Risk))
                continue;

            auto Aux    = Schedule[I];
            Schedule[I] = Schedule[J];
            Schedule[J] = Aux;

            auto Objective =
                Problem::evaluateSchedule(Model, Instance, Schedule);

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

void ILS::applyLocalSearch(Problem::Model Model,
                           const Problem::Instance &Instance,
                           std::vector<size_t> &Schedule) {
    for (size_t I = 0; scanNeighborhood(Model, Instance, Schedule) != -1; ++I)
        ;
}
