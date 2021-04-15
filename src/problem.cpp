#include <fstream>
#include <iostream>

#include "problem.h"

using namespace Problem;

struct WT {
    size_t Id, NodeId;
};

Problem::Instance Problem::loadInstance(const std::string Path) {
    std::vector<Node> Nodes;
    std::vector<Edge> Edges;
    std::ifstream InstanceFile(Path);
    size_t NumOfNodes, NumOfEdges;

    if (InstanceFile.is_open()) {
        // Read the set of nodes
        size_t Id;
        int Type, Duration, NumberOfWT;
        float Risk;
        InstanceFile >> NumOfNodes;
        for (size_t I = 0; I < NumOfNodes; ++I) {
            InstanceFile >> Id >> Type >> Duration >> NumberOfWT >> Risk;
            Nodes.push_back({Id, (NodeType)Type, (uint32_t)Duration,
                             (uint32_t)NumberOfWT, Risk});

            assert(Id == Nodes[Id].Id && "Node Id must match the index!");
            assert((Type == Origin || Type == Destination) &&
                   "Wrong Type for node!");
            assert(Duration >= 0 && "Duration is negative!");
            assert(NumberOfWT >= 0 && "NumOfWT is negative!");
            assert((0 <= Risk && Risk <= 1) && "Risk is out of range [0, 1]!");
        }

        // Read the set of edges
        size_t IdU, IdV;
        InstanceFile >> NumOfEdges;
        for (size_t I = 0; I < NumOfEdges; ++I) {
            InstanceFile >> IdU >> IdV;
            Edges.push_back({Nodes[IdU], Nodes[IdV]});
        }

        InstanceFile.close();
    } else {
        std::cerr << "error: unable to open path " << Path << "\n";
        abort();
    }
    return Instance(NumOfNodes, NumOfEdges, Nodes, Edges);
}

uint32_t Problem::evaluateSchedule(const Instance &Instance,
                                   const std::vector<size_t> &Schedule) {
    const auto Q = Instance.TotalNumOfWT();
    std::vector<WT> WTs(Q);
    std::vector<uint32_t> StartTime(Instance.NumOfNodes, 0);
    std::vector<uint32_t> CompletionTime(Instance.NumOfNodes, 0);

    // Set the starting node of every WT sequentially in order of origins.
    size_t WTId = 0;
    for (const auto OId : Instance.GetOriginsIds()) {
        for (uint32_t I = 0; I < Instance.Nodes[OId].NumberOfWT; ++I) {
            WTs[WTId].Id     = WTId;
            WTs[WTId].NodeId = OId;
            ++WTId;
        }
    }

    const auto &Duration = Instance.GetDurations();

    for (uint32_t Period = 1;; ++Period) {

        for (auto &W : WTs) {

            if (CompletionTime[W.NodeId] > Period)
                continue;

            for (const auto NodeId : Schedule) {

                if (Instance.Nodes[NodeId].isOrigin()) {
                    std::cout << "error: scheduled NodeId `" << NodeId
                              << "` is an origin\n";
                }

                else if (CompletionTime[NodeId] > 0)
                    continue;

                StartTime[NodeId] =
                    Period + Instance.DistMatrix[W.NodeId][NodeId];
                CompletionTime[NodeId] = StartTime[NodeId] + Duration[NodeId];

                W.NodeId = NodeId;
                break;
            }
        }

        // Every node have been cleaned
        if (CompletionTime[Schedule.back()] > 0)
            break;
    }

    auto Makespan =
        std::max_element(CompletionTime.begin(), CompletionTime.end());

    return *Makespan;
}

std::vector<std::vector<uint32_t>>
Problem::GetDistanceMatrix(const std::vector<Node> &Nodes,
                           const std::vector<Edge> &Edges) {
    const size_t Size = Nodes.size();

    // Creates a 2D vector of uint32_t
    std::vector<std::vector<uint32_t>> DistMatrix(
        Size, std::vector<uint32_t>(Size, Problem::M));

    // Populates the diagonal with 0's
    for (size_t I = 0; I < DistMatrix.size(); ++I)
        DistMatrix[I][I] = 0;

    // Populates the matrix with the direct edges
    for (const auto &Edge : Edges) {
        DistMatrix[Edge.U.Id][Edge.V.Id] = Edge.Weight;
        DistMatrix[Edge.V.Id][Edge.U.Id] = Edge.Weight;
    }

    // Uses the Floyd-Warshall algorithm to calculate the distances
    // between every pair of nodes
    for (size_t K = 0; K < DistMatrix.size(); ++K)
        for (size_t I = 0; I < DistMatrix.size(); ++I)
            for (size_t J = 0; J < DistMatrix.size(); ++J)
                if (DistMatrix[I][J] > DistMatrix[I][K] + DistMatrix[K][J])
                    DistMatrix[I][J] = DistMatrix[I][K] + DistMatrix[K][J];

    return DistMatrix;
}
