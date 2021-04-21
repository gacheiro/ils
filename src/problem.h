#ifndef PROBLEM_H
#define PROBLEM_H

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace Problem {

enum Model { MinMakespan, MinWeightedCompletionTime };
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

Instance loadInstance(const std::string);

double evaluateSchedule(Model, const Instance &, const std::vector<size_t> &);

static inline void repairSchedule(const Instance &Instance,
                                  std::vector<size_t> &Schedule) {
    std::stable_sort(
        Schedule.begin(), Schedule.end(), [&](size_t UId, size_t VId) -> bool {
            return Instance.Nodes[UId].Risk > Instance.Nodes[VId].Risk;
        });
}

} // namespace Problem

#endif
