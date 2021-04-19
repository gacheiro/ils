#ifndef ILS_H
#define ILS_H

#include "problem.h"

namespace ILS {

// Global flag to terminate the execution when the time limit has exceeded
extern bool TIME_LIMIT_EXCEEDED;

Problem::Solution solveInstance(Problem::Model, const Problem::Instance &,
                                float);

void applyLocalSearch(Problem::Model, const Problem::Instance &,
                      std::vector<size_t> &);
} // namespace ILS

#endif
