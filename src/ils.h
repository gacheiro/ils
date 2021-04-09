#ifndef ILS_H
#define ILS_H

#include "problem.h"

namespace ILS {
Problem::Solution solveInstance(const Problem::Instance &, float, int);
void applyLocalSearch(const Problem::Instance &, std::vector<size_t> &);
} // namespace ILS

#endif
