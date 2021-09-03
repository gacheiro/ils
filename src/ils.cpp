#include <random>

#include "ils.h"
#include "problem.h"

using namespace ILS;

bool ILS::TIME_LIMIT_EXCEEDED   = false;
float ILS::RELAXATION_THRESHOLD = 0;

static std::default_random_engine RandomGenerator;

static void applyPerturbation(const Problem::Instance &, std::vector<size_t> &,
                              float);
static int32_t scanNeighborhood(Problem::Model Model, const Problem::Instance &,
                                std::vector<size_t> &);
static bool canSwap(const Problem::Instance &, const std::vector<size_t> &,
                    size_t I, size_t J, float);

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
        applyPerturbation(Instance, CandidateSchedule, PerturbationStrength);

        applyLocalSearch(Model, Instance, CandidateSchedule);

        // Evaluates the schedule with perturbation
        auto CandidateObjective =
            Problem::evaluateSchedule(Model, Instance, CandidateSchedule);

        if (CurrentObjective > CandidateObjective) {
            CurrentObjective = CandidateObjective;
            CurrentSchedule  = CandidateSchedule;
        }
    } while (!ILS::TIME_LIMIT_EXCEEDED);

    Problem::AssertPiorityRules(Instance, CurrentSchedule,
                                ILS::RELAXATION_THRESHOLD);

    return Problem::Solution(CurrentObjective, CurrentSchedule);
}

static void applyPerturbation(const Problem::Instance &Instance,
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

static int32_t scanNeighborhood(Problem::Model Model,
                                const Problem::Instance &Instance,
                                std::vector<size_t> &Schedule) {
    const auto Size = Schedule.size();
    auto CurrentObjective =
        Problem::evaluateSchedule(Model, Instance, Schedule);

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
    while (scanNeighborhood(Model, Instance, Schedule) != -1)
        if (ILS::TIME_LIMIT_EXCEEDED)
            return;
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
