#ifndef PROBLEM_H
#define PROBLEM_H

#include <algorithm>
#include <cassert>
#include <climits>
#include <iostream>
#include <string>
#include <vector>

namespace Problem {

enum NodeType { Origin, Destination };

const int M    = INT_MAX;
const auto EPS = 1e-7;

struct Config {
    float RelaxationThreshold;
    bool SetupTimes;
};

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
    uint32_t Weight;
};

std::vector<std::vector<uint32_t>> GetDistanceMatrix(const std::vector<Node> &,
                                                     const std::vector<Edge> &);

struct Instance {
    size_t NumOfNodes;
    size_t NumOfEdges;
    std::vector<Node> Nodes;
    std::vector<Edge> Edges;
    std::vector<std::vector<uint32_t>> DistMatrix;
    float RelaxationThreshold{.0f};

    Instance(size_t _NumOfNodes, size_t _NumOfEdges, std::vector<Node> _Nodes,
             std::vector<Edge> _Edges, float _RelaxationThreshold)
        : NumOfNodes{_NumOfNodes}, NumOfEdges{_NumOfEdges}, Nodes{_Nodes},
          Edges{_Edges}, RelaxationThreshold{_RelaxationThreshold} {

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
  private:
    Problem::Instance Instance;
    uint32_t Makespan;
    std::vector<size_t> Schedule;
    std::vector<uint32_t> StartTime;
    std::vector<uint32_t> CompletionTime;

  public:
    Solution(Problem::Instance _Instance, std::vector<size_t> _Schedule)
        : Instance(_Instance), Schedule{_Schedule} {
        StartTime.resize(Instance.NumOfNodes);
        CompletionTime.resize(Instance.NumOfNodes);
    }

    size_t Size() { return Schedule.size(); }
    uint32_t GetMakespan();
    bool SwapTasks(size_t NodeIdA, size_t NodeIdB);
    bool IsFeasible();
    void PrintSchedule();
};

//
// TODO: RelaxationThreshold shouldn't be a param here
//
/// Loads a problem's instance.
///
/// Typical usage:
/// \code
///   Problem::Instance Instance = loadInstance(InstancePath);
/// \endcode
///
/// \param InstancePath the path of the instance to load.
/// \param RelaxationThreshold the value of the relaxation threshold.
///
/// \returns the loaded Problem::Instance.
Instance loadInstance(std::string InstancePath, Config Config);

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

/// Checks if the precedence rule between two risks can be relaxed.
///
/// \param Instance the problem's instance
/// \param NodeIdA the risk associated to the first node.
/// \param NodeIdB the risk associated to the second node.
///
/// \returns true if the precendences can be relaxed or false otherwise.
bool canSwap(const Problem::Instance &Instance,
             const std::vector<size_t> &Schedule, size_t I, size_t J);
} // namespace Problem
#endif
