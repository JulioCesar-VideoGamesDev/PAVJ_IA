#pragma once

#include "CoreMinimal.h"

struct FGridConnection
{
    float Cost = 1.f;

    struct FGridNode* ToNode = nullptr;
};

struct FGridNode
{
    int X = 0;
    int Y = 0;

    bool bWalkable = true;

    float TileCost = 1.f;

    TArray<FGridConnection> Connections;
};

class FGrid
{
    TArray<FGridNode> GridNodeArray;
};