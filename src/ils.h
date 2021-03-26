#ifndef ILS_H
#define ILS_H

#include <string>
#include <vector>

#include "problem.h"

namespace ILS {
Problem::Solution solveInstance(const Problem::Instance &, double, int);
Problem::Solution applyLocalSearch(const Problem::Instance &,
                                   Problem::Solution &);
} // namespace ILS

#endif
