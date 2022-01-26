#ifndef ILS_H
#define ILS_H

#include <random>

#include "problem.h"

namespace ILS {

// Keep all config variables together;
struct Config {
    float RelaxationThreshold;
    float PerturbationStrength;
    long int Evaluations;
    int RandomSeed;
};

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
                                Config Config);

/// Applies a perturbation to a solution. The solution is modified in place.
///
/// \param Solution the solution to apply the local search.
/// \param PerturbationStrength perturbation strength param.
void applyPerturbation(Problem::Solution &Solution, float PerturbationStrength,
                       std::default_random_engine &RandomGenerator);

/// Applies a local search to a solution. The solution is modified in place.
///
/// Typical usage:
/// \code
///   applyLocalSearch(Problem::MinMakespan, Instance, Schedule, NULL);
/// \endcode
///
/// \param Solution solution to apply local search to.
/// \param TLE time limit exceeded flag.
///        Should be handled by another thread.
void applyLocalSearch(Problem::Solution &Solution, long int &Evaluations);

} // namespace ILS
#endif
