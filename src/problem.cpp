#include <fstream>
#include <iostream>

#include "problem.h"

using namespace Problem;

Problem::Instance Problem::loadInstance(const char *Path) {
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
        std::cerr << "unable to open path " << Path << "\n";
        abort();
    }
    return Instance(NumOfNodes, NumOfEdges, Nodes, Edges);
}

struct WT {
    size_t Id, NodeId;
};

uint32_t Problem::evaluateSchedule(const Instance &Instance,
                                   const std::vector<Node> &Schedule) {
    const auto Q = Instance.TotalNumOfWT();
    std::vector<WT> WTs(Q);
    std::vector<uint32_t> StartTime(Instance.NumOfNodes, 0);
    std::vector<uint32_t> CompletionTime(Instance.NumOfNodes, 0);

    // Set the starting node of every WT sequentially in order of origins.
    size_t WTId = 0;
    for (const auto OId : Instance.GetOriginIds()) {
        for (uint32_t I = 0; I < Instance.Nodes[OId].NumberOfWT; ++I) {
            WTs[WTId].Id     = WTId;
            WTs[WTId].NodeId = OId;
            ++WTId;
        }
    }

    const auto T           = Instance.TotalNumOfPeriods();
    const auto &DistMatrix = GetDistanceMatrix(Instance.Nodes, Instance.Edges);
    const auto &Duration   = Instance.GetDurations();
    for (uint32_t Period = 1; Period <= T; ++Period)
        for (auto &W : WTs)
            for (const auto &Node : Schedule) {
                assert(!Node.isOrigin() && "Scheduled Node is an origin!");
                if (CompletionTime[Node.Id] > 0)
                    continue;

                else if (CompletionTime[W.NodeId] <= Period) {
                    if (Instance.Nodes[W.NodeId].isOrigin())
                        // Don't consider the distance between origins and
                        // destinations (bug in the model).
                        StartTime[Node.Id] = Period;
                    else
                        // Ok for the distance between destinations
                        StartTime[Node.Id] = Period + DistMatrix[W.NodeId][Node.Id];
                    CompletionTime[Node.Id] =
                        StartTime[Node.Id] + Duration[Node.Id];
                    W.NodeId = Node.Id;
                }
            }

    auto Makespan =
        std::max_element(CompletionTime.begin(), CompletionTime.end());

    for (const auto S : StartTime)
        std::cout << S << " ";
    std::cout << std::endl;
    for (const auto C : CompletionTime)
        std::cout << C << " ";
    std::cout << std::endl;
    std::cout << "Makespan = " << *Makespan << std::endl;
    return *Makespan;
}

std::vector<std::vector<uint32_t>>
Problem::GetDistanceMatrix(const std::vector<Node> Nodes,
                           const std::vector<Edge> Edges) {
    const size_t Size = Nodes.size();
    // Creates a 2D vector of uint32_t
    std::vector<std::vector<uint32_t>> DistMatrix(
        Size, std::vector<uint32_t>(Size, 999));

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
