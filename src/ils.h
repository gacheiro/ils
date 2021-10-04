#ifndef ILS_H
#define ILS_H

#include <random>

#include "problem.h"

namespace ILS {

// TODO:
// - `RELAXATION_THRESHOLD` should be a function parameter;
// - Function `applyLocalSearch` should accept a solution instead of a schedule
//   and return the best objective function found during the procedure;

extern float RELAXATION_THRESHOLD;
extern std::default_random_engine RandomGenerator;

/// Solves a problem's instance.
///
/// Typical usage:
/// \code
///   Problem::Solution Sol = solveInstance(Problem::MinMakespan, Instance,
///                                         0.5);
///   std::cout << Solution.Objective << "\n";
/// \endcode
///
/// \param Instance the problem's instance to solve.
/// \param PerturbationStrength ILS' perturbation strength param.
/// \param TimeLimit execution time limit (in seconds).
///
/// \returns a Problem::Solution for the instance.
Problem::Solution solveInstance(const Problem::Instance &Instance,
                                float PerturbationStrength, uint32_t TimeLimit);

/// Applies a perturbation to a schedule. The schedule is modified in place.
///
/// \param Instance the problem's instance to solve.
/// \param Schedule the schedule to apply the local search.
/// \param PerturbationStrength perturbation strength param.
void applyPerturbation(const Problem::Instance &Instance,
                       std::vector<size_t> &Schedule,
                       float PerturbationStrength);

/// Applies a local search to a schedule. The schedule is modified in place.
///
/// Typical usage:
/// \code
///   applyLocalSearch(Problem::MinMakespan, Instance, Schedule, NULL);
/// \endcode
///
/// \param Instance the problem's instance to solve.
/// \param Schedule the schedule to apply the local search.
/// \param TLE time limit exceeded flag.
///            Should be handled by another thread.
void applyLocalSearch(const Problem::Instance &Instance,
                      std::vector<size_t> &Schedule, const bool *TLE);

} // namespace ILS
#endif
