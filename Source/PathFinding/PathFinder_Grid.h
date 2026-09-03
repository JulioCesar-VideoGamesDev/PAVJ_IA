#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PathFinder_Grid.generated.h"

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
class MPV_PRACTICAS_API UPathFinder_Grid : public UObject
{
    GENERATED_BODY()

public:
    UPathFinder_Grid();

    UPROPERTY(EditDefaultsOnly, Category = "Pathfinding")
    FString GridMap_FilePath{ "" };

    UPROPERTY(EditDefaultsOnly, Category = "Pathfinding")
    FString CostConfig_FilePath{ "" };

    // Finds a path between two points.
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector> FindPath(FVector StartLocation, FVector EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector> GetLastPath() const { return LastPath; }

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void DrawPath(const TArray<FVector>& Path, FColor Color = FColor::Yellow);

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


    // Last calculated path.
    TArray<FVector> LastPath;

    // Cost maps for the characters.
    TMap<char, float> CostMap;

public:
    FGridCell* CurrentStart{ nullptr };
    FGridCell* CurrentEnd{ nullptr };

// TODO: EXTRACT ALL BELOW TO A NEW class Grid;
public:

    // Loads the grid from a file.
    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool LoadGridFromFile(FString FilePath, FString CostConfigPath = "");

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void SetupDefaultGrid(int32 GridSize_CellCountX = 10, int32 GridSize_CellCountY = 10, float CellSize = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void DrawGrid(bool bDrawCosts = true);

    bool GridDrawn{ false };

    UFUNCTION(BlueprintCallable, Category = "Grid")
    FVector GetGridSize() const { return GridSize; }

    UFUNCTION(BlueprintCallable, Category = "Grid")
    float GetCellSize() const { return CellSize; }

    UFUNCTION(BlueprintCallable, Category = "Grid")
    FVector GetGridOrigin() const { return GridOrigin; }

    // Returns a cell based on a world location.
    FGridCell* GetCellAtLocation(FVector Location);

    // Returns a cell based on a grid position/coordinates.
    FGridCell* GetCellAtGrid(FIntPoint GridPos);

private:

    TArray<FGridCell> Grid;

    FIntPoint GridSize_CellCount{ FIntPoint(0, 0) };
    
    float CellSize{ 100.f };
    
    // Origin of the grid in world coordinates.
    FVector GridOrigin{ FVector(-500.f, 0.f, -400.f) };

    FVector GridSize{ FVector(
        GridSize_CellCount.X * CellSize - GridOrigin.X,
        GridOrigin.Y,
        GridSize_CellCount.Y * CellSize - GridOrigin.Z)
    };
};