#ifndef PROBLEM_H
#define PROBLEM_H

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

namespace Problem {

enum NodeType { Origin, Destination };

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

struct Instance {
    size_t NumOfNodes;
    size_t NumOfEdges;
    std::vector<Node> Nodes;
    std::vector<Edge> Edges;

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
    }

    std::vector<size_t> GetOriginIds() const {
        std::vector<size_t> Ids;
        for (const auto &Node : Nodes)
            if (Node.Type == Origin)
                Ids.push_back(Node.Id);
        return Ids;
    }

    std::vector<Node> GetDestinations() const {
        std::vector<Node> Destinations;
        for (const auto &Node : Nodes)
            if (Node.Type == Destination)
                Destinations.push_back(Node);
        return Destinations;
    }

    std::vector<uint32_t> GetDurations() const {
        std::vector<uint32_t> Duration(NumOfNodes);
        for (const auto &Node : Nodes)
            Duration[Node.Id] = Node.Duration;
        return Duration;
    }

    uint32_t TotalNumOfPeriods() const {
        uint32_t Sum = 0;
        for (const auto &Node : Nodes)
            Sum += Node.Duration;
        // NOTE: Naive way to set an upper bound for the makespan.
        // This can fail sometimes :(
        return Sum * 2;
    }

    uint32_t TotalNumOfWT() const {
        uint32_t Sum = 0;
        for (const auto &Node : Nodes)
            Sum += Node.NumberOfWT;
        return Sum;
    }
};

struct Solution {
    uint32_t Objective;
    std::vector<Node> Schedule;

    Solution(uint32_t _Objective, std::vector<Node> _Schedule)
        : Objective(_Objective), Schedule{_Schedule} {}
};

Instance loadInstance(const char *);
uint32_t evaluateSchedule(const Instance &, const std::vector<Node> &);
std::vector<std::vector<uint32_t>> GetDistanceMatrix(const std::vector<Node>,
                                                     const std::vector<Edge>);

static inline void repairSchedule(std::vector<Node> &Schedule) {
    std::stable_sort(Schedule.begin(), Schedule.end(),
              [&](Problem::Node U, Problem::Node V) -> bool {
                  return U.Risk > V.Risk;
              });
}

} // namespace Problem

#endif
