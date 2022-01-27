#include <fstream>

#include "problem.h"

using namespace Problem;

struct WT {
    size_t Id, NodeId;
};

Problem::Instance Problem::loadInstance(std::string InstancePath,
                                        Problem::Config Config) {
    std::vector<Node> Nodes;
    std::vector<Edge> Edges;
    std::ifstream InstanceFile(InstancePath);
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
        uint32_t DefaultWeight = (Config.SetupTimes ? 1 : 0);
        size_t IdU, IdV;
        InstanceFile >> NumOfEdges;
        for (size_t I = 0; I < NumOfEdges; ++I) {
            InstanceFile >> IdU >> IdV;
            Edges.push_back({Nodes[IdU], Nodes[IdV], DefaultWeight});
        }

        InstanceFile.close();
    } else {
        std::cerr << "error: unable to open path " << InstancePath << "\n";
        abort();
    }
    return Instance(NumOfNodes, NumOfEdges, Nodes, Edges,
                    Config.RelaxationThreshold);
}

std::vector<size_t> Problem::constructSchedule(Instance Instance) {
    std::vector<size_t> Schedule = Instance.GetDestinationsIds();
    // Sort nodes by risk in descending order
    std::stable_sort(
        Schedule.begin(), Schedule.end(), [&](size_t UId, size_t VId) -> bool {
            return Instance.Nodes[UId].Risk > Instance.Nodes[VId].Risk;
        });
    return Schedule;
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

uint32_t Problem::Solution::GetMakespan() {
    const auto Q = Instance.TotalNumOfWT();
    std::vector<WT> WTs(Q);
    for (size_t I = 0; I < Instance.NumOfNodes; ++I) {
        StartTime[I]      = 0;
        CompletionTime[I] = 0;
    }

    // Sets the starting node of every WT sequentially in order of origins.
    size_t WTId = 0;
    for (const auto OId : Instance.GetOriginsIds()) {
        for (uint32_t I = 0; I < Instance.Nodes[OId].NumberOfWT; ++I) {
            WTs[WTId].Id     = WTId;
            WTs[WTId].NodeId = OId;
            ++WTId;
        }
    }

    // Note: this is quite inefficient O(n^2)
    auto SchedulePtr = Schedule.begin();
    for (uint32_t Period = 0;;) {

        if (SchedulePtr == Schedule.end())
            break;

        auto NodeId = *SchedulePtr;
        // Loops through available teams and chooses
        // the one which provides the earliast finish date
        int EarliestFinishTime  = M;
        auto EarliestFinishTeam = -1;
        for (size_t I = 0; I < WTs.size(); ++I) {
            if (CompletionTime[WTs[I].NodeId] > Period)
                continue;

            int FinishTime = Period +
                             Instance.DistMatrix[WTs[I].NodeId][NodeId] +
                             Instance.Nodes[NodeId].Duration;

            if (FinishTime < EarliestFinishTime) {
                EarliestFinishTime = FinishTime;
                EarliestFinishTeam = I;
            }
        }

        if (EarliestFinishTeam != -1) {
            StartTime[NodeId] = CompletionTime[WTs[EarliestFinishTeam].NodeId];
            CompletionTime[NodeId]         = EarliestFinishTime;
            WTs[EarliestFinishTeam].NodeId = NodeId;
            ++SchedulePtr;
        } else
            ++Period;
    }

    for (auto NodeId : Schedule)
        assert(CompletionTime[NodeId] && !Instance.Nodes[NodeId].isOrigin());

    Makespan = *std::max_element(CompletionTime.begin(), CompletionTime.end());
    return Makespan;
}

bool Problem::Solution::SwapTasks(size_t NodeIdA, size_t NodeIdB) {
    if (!canSwap(Instance, Schedule, NodeIdA, NodeIdB))
        return false;

    auto Aux          = Schedule[NodeIdA];
    Schedule[NodeIdA] = Schedule[NodeIdB];
    Schedule[NodeIdB] = Aux;

    return true;
}

bool Problem::canSwap(const Problem::Instance &Instance,
                      const std::vector<size_t> &Schedule, size_t I, size_t J) {
    assert(I <= J && "Range [I, J] is invalid!");
    assert(Schedule.size() > 0 && "Schedule is empty!");

    auto &Nodes             = Instance.Nodes;
    auto HighestRisk        = Nodes[Schedule[I]].Risk;
    size_t HighestRiskIndex = I;

    // Gets the highest node in the range (I, J)
    for (auto K = I + 1; K < J; ++K) {
        if (Nodes[Schedule[K]].Risk > HighestRisk) {
            HighestRisk      = Nodes[Schedule[K]].Risk;
            HighestRiskIndex = K;
        }
    }

    return Problem::canRelaxPriority(Nodes[Schedule[HighestRiskIndex]].Risk,
                                     Nodes[Schedule[J]].Risk,
                                     Instance.RelaxationThreshold);
}

// NOTE: this function fails for some feasible solutions
bool Problem::Solution::IsFeasible() {
    /*
    auto Size_ = Size();
    for (size_t I = 0; I < Size_ - 1; ++I) {
        for (size_t J = I + 1; J < Size_; ++J)
            // https://stackoverflow.com/questions/4548004/how-to-correctly-and-standardly-compare-floats
            if (!(fabs(Instance.Nodes[Schedule[I]].Risk -
                      Instance.Nodes[Schedule[J]].Risk) +
                      Instance.RelaxationThreshold >=
                  EPS))
                return false;
    }
    */
    return true;
}

void Problem::Solution::PrintSchedule() {
    std::cout << "Makespan: " << GetMakespan() << '\n';
    for (auto NodeId : Schedule)
        std::cout << CompletionTime[NodeId] << ' ';
    std::cout << '\n';
}
