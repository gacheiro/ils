#ifndef PROBLEM_H
#define PROBLEM_H

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace Problem {

enum NodeType { Origin, Destination };

const int M = 999;

struct Node {
    size_t Id;
    NodeType Type;
    uint32_t Duration;
    uint32_t NumberOfWT;
    float Risk;

    Node(size_t _Id, NodeType _Type, uint32_t _Duration, uint32_t _NumberOfWT,
         float _Risk)
        : Id{_Id}, Type{_Type}, Duration{_Duration},
          NumberOfWT{_NumberOfWT}, Risk{_Risk} {}

    bool isOrigin() const { return Type == Origin; }
};

struct Edge {
    Node U;
    Node V;
    uint32_t Weight{1};

    Edge(Node _U, Node _V) : U{_U}, V{_V} {}
};

std::vector<std::vector<uint32_t>> GetDistanceMatrix(const std::vector<Node> &,
                                                     const std::vector<Edge> &);

struct Instance {
    size_t NumOfNodes;
    size_t NumOfEdges;
    std::vector<Node> Nodes;
    std::vector<Edge> Edges;
    std::vector<std::vector<uint32_t>> DistMatrix;

    Instance(size_t _NumOfNodes, size_t _NumOfEdges, std::vector<Node> _Nodes,
             std::vector<Edge> _Edges)
        : NumOfNodes{_NumOfNodes},
          NumOfEdges{_NumOfEdges}, Nodes{_Nodes}, Edges{_Edges} {

        if (NumOfNodes != Nodes.size()) {
            std::cerr << "Nodes.size() differs from NumOfNodes. It's "
                      << Nodes.size() << " when it should be " << NumOfNodes
                      << "\n";
            abort();
        }

        DistMatrix = Problem::GetDistanceMatrix(Nodes, Edges);
    }

    std::vector<size_t> GetOriginsIds() const {
        std::vector<size_t> Ids;
        for (const auto &Node : Nodes)
            if (Node.Type == Origin)
                Ids.push_back(Node.Id);
        return Ids;
    }

    std::vector<size_t> GetDestinationsIds() const {
        std::vector<size_t> Ids;
        for (const auto &Node : Nodes)
            if (Node.Type == Destination)
                Ids.push_back(Node.Id);
        return Ids;
    }

    uint32_t TotalNumOfWT() const {
        uint32_t Sum = 0;
        for (const auto &Node : Nodes)
            Sum += Node.NumberOfWT;
        return Sum;
    }
};

struct Solution {
    double Objective;
    std::vector<size_t> Schedule;

    Solution(double _Objective, std::vector<size_t> _Schedule)
        : Objective(_Objective), Schedule{_Schedule} {}
};

/// Loads a problem's instance.
///
/// Typical usage:
/// \code
///   Problem::Instance Instance = loadInstance(InstancePath);
/// \endcode
///
/// \param InstancePath the path of the instance to load.
///
/// \returns the loaded Problem::Instance.
Instance loadInstance(const std::string);

/// Constructs a feasible schedule for an instance.
/// Used as a constructive heuristic.
///
/// Typical usage:
/// \code
///   auto Schedule = Problem::constructSchedule(Instance);
/// \endcode
///
/// \param Instance the problem's instance to solve.
///
/// \returns a valid schedule for the problem.
std::vector<size_t> constructSchedule(Instance Instance);

/// Evaluates the objective function of a schedule.
///
/// Typical usage:
/// \code
///   auto Objective = Problem::evaluateSchedule(Instance, Schedule);
/// \endcode
///
/// \param Instance the problem's instance.
/// \param Schedule the schedule to be evaluated.
///
/// \returns the value of the objective function for the schedule.
uint32_t evaluateSchedule(const Instance &Instance,
                          const std::vector<size_t> &Schedule);

/// Checks if the precedence rule between two risks can be relaxed.
///
/// \param RiskA the risk associated to the first node.
/// \param RiskB the risk associated to the second node.
/// \param RelaxationThreshold the value of the relaxation threshold.
///
/// \returns true if the precendences can be relaxed or false otherwise.
static inline bool canRelaxPriority(float RiskA, float RiskB,
                                    float RelaxationThreshold) {
    return RiskA <= RiskB + RelaxationThreshold;
}

/// Checks if a schedule is valid according to the priority rules.
// If th schedule is invalid, throws an assertion error.
///
/// \param Instance the problem's instance.
/// \param Schedule the schedule to be evaluated.
/// \param RelaxationThreshold the value of the relaxation threshold.
static inline void AssertPiorityRules(const Instance &Instance,
                                      const std::vector<size_t> &Schedule,
                                      float RelaxationThreshold) {
    for (size_t I = 0; I < Schedule.size() - 1; ++I) {
        for (size_t J = I + 1; J < Schedule.size(); ++J)
            assert(Instance.Nodes[Schedule[I]].Risk >=
                   Instance.Nodes[Schedule[J]].Risk - RelaxationThreshold);
    }
}

} // namespace Problem

#endif
