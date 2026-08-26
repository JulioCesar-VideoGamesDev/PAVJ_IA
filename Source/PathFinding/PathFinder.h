#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PathFinder.generated.h"

// Struct to represent a tile in the Grid
USTRUCT(BlueprintType)
struct FGridCell
{
    GENERATED_BODY()

    // Position in the grid.
    UPROPERTY(BlueprintReadWrite)
    FIntPoint GridPosition{ FIntPoint(-1, -1) };

    UPROPERTY(BlueprintReadWrite)
    FVector WorldLocation{ FVector::ZeroVector };

    UPROPERTY(BlueprintReadWrite)
    bool bIsWalkable{ true };

    UPROPERTY(BlueprintReadWrite)
    float Cost{ 1.f };

    float GCost{ 0.f };  // Cost from the begining
    float HCost{ 0.f };  // Heucaristic to the objective
    float FCost{ 0.f };  // Total cost

    FGridCell* Parent{ nullptr };

    // State for the algorithm
    bool bIsInOpenSet{ false };
    bool bIsInClosedSet{ false };

    FGridCell() {}

    FGridCell(FIntPoint InPos, FVector InWorldLoc, float InCost = 1.0f, bool InWalkable = true)
        : GridPosition(InPos)
        , WorldLocation(InWorldLoc)
        , bIsWalkable(InWalkable)
        , Cost(InCost)
        , GCost(0.0f)
        , HCost(0.0f)
        , FCost(0.0f)
        , Parent(nullptr)
        , bIsInOpenSet(false)
        , bIsInClosedSet(false)
    {}
};

UCLASS(BlueprintType)
class MPV_PRACTICAS_API UPathFinder : public UObject
{
    GENERATED_BODY()

public:
    UPathFinder();

    // Finds a path between two points.
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector> FindPath(FVector StartLocation, FVector EndLocation);

    // Loads the grid from a file.
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool LoadGridFromFile(FString FilePath, FString CostConfigPath = "");

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void SetupDefaultGrid(int32 GridSizeX = 10, int32 GridSizeY = 10, float CellSize = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void DrawGrid(bool bDrawCosts = true);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void DrawPath(const TArray<FVector>& Path, FColor Color = FColor::Yellow);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector> GetLastPath() const { return LastPath; }

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    FIntPoint GetGridSize() const { return GridSize; }

    // Returns a cell based on a world location.
    FGridCell* GetCellAtLocation(FVector Location);

    // Returns a cell based on a grid position/coordinates.
    FGridCell* GetCellAtGrid(FIntPoint GridPos);

    class UWorld* World = nullptr;

protected:

    // Implementation of the A* algorithm.
    TArray<FVector> AStar(FGridCell* StartCell, FGridCell* EndCell);

    // Reconstruct the path from the EndCell to the StartCell.
    void ReconstructPath(FGridCell* EndCell, TArray<FVector>& OutPath);

    // Calculates the heuristic Manhattan Distance (only 4 directions).
    float Heuristic(FGridCell* A, FGridCell* B);

    // GetNeighbors in the 4 directions.
    TArray<FGridCell*> GetNeighbors(FGridCell* Cell);

private:

    TArray<FGridCell> Grid;

    FIntPoint GridSize{ FIntPoint(0, 0) };

    // Last calculated path.
    TArray<FVector> LastPath;

    // Cost maps for the characters.
    TMap<char, float> CostMap;

public:

    float CellSize{ 100.f };

    FGridCell* CurrentStart{ nullptr };
    FGridCell* CurrentEnd{ nullptr };
    
    // Origin of the grid in world coordinates.
    FVector GridOrigin{ FVector::ZeroVector };
};