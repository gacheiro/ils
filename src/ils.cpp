#include <iostream>

#include "ils.h"
#include "problem.h"

using namespace ILS;

static inline void applyPerturbation(std::vector<Problem::Node> &Solution,
                                     double PerturbationStrength);

Problem::Solution ILS::solveInstance(const Problem::Instance &Instance,
                                     double PerturbationStrength,
                                     int NumOfIterations) {

    // Sort vector by risk in descending order
    std::vector<Problem::Node> CurrentSchedule = Instance.GetDestinations();
    Problem::repairSchedule(CurrentSchedule);

    auto CurrentObjective =
        Problem::evaluateSchedule(Instance, CurrentSchedule);

    do {
        // Copies the schedule and evaluates the perturbated schedule
        std::vector<Problem::Node> CandidateSchedule = CurrentSchedule;
        applyPerturbation(CandidateSchedule, PerturbationStrength);
        // Sort vector by risk in descending order
        Problem::repairSchedule(CandidateSchedule);

        // Evaluates the schedule with perturbation
        auto CandidateObjective =
            Problem::evaluateSchedule(Instance, CandidateSchedule);

        if (CurrentObjective > CandidateObjective) {
            // Updates the current solution
            CurrentObjective = CandidateObjective;
            CurrentSchedule  = CandidateSchedule;
        }

        std::cout << "ILS: iteration: " << CurrentObjective << " objective\n";
    } while (--NumOfIterations > 0);

    return Problem::Solution(CurrentObjective, CurrentSchedule);
}

static inline void applyPerturbation(std::vector<Problem::Node> &Schedule,
                                     double PerturbationStrength) {
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

Problem::Solution ILS::applyLocalSearch(const Problem::Instance &Instance,
                                        Problem::Solution &Solution) {}
