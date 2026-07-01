#pragma once

#include "CoreMinimal.h"
#include "Grid.h"

class AAICharacter;

class MPV_PRACTICAS_API FPathfinder
{
protected:
    struct FPathNode
    {
        FGridNode* Node = nullptr;

        FPathNode* Parent = nullptr;

        float G = 0.f;

        float H = 0.f;

        float F() const
        {
            return G + H;
        }
    };

public:
    FPathfinder();
    
    ~FPathfinder();

    // Lee el txt y construye el grafo de navegación
    void LoadGrid(FString Filename, float TileWidth, float TileHeight, FVector GridOrigin);

    TArray<FVector> FindPath(FVector Start, FVector Goal);

    void DrawDebug(UWorld* World);

protected:

    FGridNode* GetNode(int X, int Y) const;

    FGridNode* GetClosestNode(FVector Position) const;

    FVector NodeToWorld(const FGridNode* Node) const;

private:
    float Heuristic(FGridNode* A, FGridNode* B);



    TArray<FVector> BuildPath(FPathNode* GoalNode);

    int GetLowestFIndex(const TArray<FPathNode>& OpenList);

    bool IsClosed(const TArray<FPathNode>& ClosedList, FGridNode* Node);

    int FindOpenNodeIndex(TArray<FPathNode>& OpenList, FGridNode* Node);

    TArray<FGridNode*> GetNeighbours(FGridNode* Node);

protected:

    FGrid Grid;

    // Node = Grid[Y * Width + X];

    int Width = 0;
    int Height = 0;

    float CellWidth = 100.f;
    float CellHeight = 100.f;

    FVector Origin;

private:

    TArray<FVector> CurrentPath;

    FVector LastStart;
    FVector LastGoal;
};