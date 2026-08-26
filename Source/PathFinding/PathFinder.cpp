#include "PathFinder.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UPathFinder::UPathFinder()
{
    // Initialize map with default values
    CostMap.Add('.', 1.f);
    CostMap.Add('#', 0.f);  // Obstacle
}

TArray<FVector> UPathFinder::FindPath(FVector StartLocation, FVector EndLocation)
{
    LastPath.Empty();

    // We get the cells of the Start and the End.
    CurrentStart = GetCellAtLocation(StartLocation);
    CurrentEnd = GetCellAtLocation(EndLocation);

    // Validate that they exist and that they are walkable.
    if (!CurrentStart || !CurrentEnd)
    {
        UE_LOG(LogTemp, Warning, TEXT("Pathfinder: Start or End out of grid."));
        return LastPath;
    }

    if (!CurrentStart->bIsWalkable || !CurrentEnd->bIsWalkable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Pathfinder: Start or End are not Walkable."));
        return LastPath;
    }

    // Execure A* algorithm.
    LastPath = AStar(CurrentStart, CurrentEnd);

    if (LastPath.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Pathfinder: Path found with %d waypoints"), LastPath.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Pathfinder: No path found"));
    }

    return LastPath;
}


// Implementation of the A* algorithm.
TArray<FVector> UPathFinder::AStar(FGridCell* StartCell, FGridCell* EndCell)
{
    // Reset ALL nodes
    for (FGridCell& Cell : Grid)
    {
        Cell.GCost = INFINITY;
        Cell.HCost = 0.f;
        Cell.FCost = INFINITY;
        Cell.Parent = nullptr;
        Cell.bIsInOpenSet = false;
        Cell.bIsInClosedSet = false;
    }

    TArray<FGridCell*> OpenSet;     // Border
    TArray<FGridCell*> ClosedSet;   // Explored nodes

    // Setup StartCell
    StartCell->GCost = 0.f;
    StartCell->HCost = Heuristic(StartCell, EndCell);
    StartCell->FCost = StartCell->GCost + StartCell->HCost;
    StartCell->bIsInOpenSet = true;
    OpenSet.Add(StartCell);

    // Main loop of A*
    while (!OpenSet.IsEmpty())
    {
        // Get cell with the lowest cost in the Border.
        FGridCell* Current = OpenSet[0];
        int32 CurrentIndex = 0;
        for (int32 i = 1; i < OpenSet.Num(); ++i)
        {
            if (OpenSet[i]->FCost < Current->FCost)
            {
                Current = OpenSet[i];
                CurrentIndex = i;
            }
        }

        // Check if we reached the objective.
        if (Current == EndCell)
        {
            TArray<FVector> Path;
            ReconstructPath(Current, Path);
            return Path;
        }

        // Swap the cell from open to closed since we already explored it.
        OpenSet.RemoveAt(CurrentIndex);
        Current->bIsInOpenSet = false;
        Current->bIsInClosedSet = true;
        ClosedSet.Add(Current);

        // Explore neighbors
        TArray<FGridCell*> Neighbors = GetNeighbors(Current);
        for (FGridCell* Neighbor : Neighbors)
        {
            // Skip if it's already closed or is not walkable.
            if (Neighbor->bIsInClosedSet || !Neighbor->bIsWalkable)
                continue;

            // Calculate tentative cost.
            float TentativeG = Current->GCost + Neighbor->Cost;

            // If It's not in the border cells array then we add it.
            if (!Neighbor->bIsInOpenSet)
            {
                Neighbor->bIsInOpenSet = true;
                OpenSet.Add(Neighbor);
            }
            else if (TentativeG >= Neighbor->GCost)
            {
                // This path is not better.
                continue;
            }

            // Update with the best path.
            Neighbor->Parent = Current;
            Neighbor->GCost = TentativeG;
            Neighbor->HCost = Heuristic(Neighbor, EndCell);
            Neighbor->FCost = Neighbor->GCost + Neighbor->HCost;
        }
    }

    // No path was found.
    return TArray<FVector>();
}

// Reconstruct the path from the EndCell to the StartCell.
void UPathFinder::ReconstructPath(FGridCell* EndCell, TArray<FVector>& OutPath)
{
    OutPath.Empty();

    FGridCell* Current = EndCell;
    while (Current)
    {
        OutPath.Add(Current->WorldLocation);
        Current = Current->Parent;
    }

    // Invert to get the path from the begining to the end.
    for (int32 i = 0; i < OutPath.Num() / 2; ++i)
    {
        int32 j = OutPath.Num() - 1 - i;
        FVector Temp = OutPath[i];
        OutPath[i] = OutPath[j];
        OutPath[j] = Temp;
    }
}

// Calculates the heuristic Manhattan Distance (only 4 directions).
float UPathFinder::Heuristic(FGridCell* A, FGridCell* B)
{
    return FMath::Abs(A->GridPosition.X - B->GridPosition.X) +
        FMath::Abs(A->GridPosition.Y - B->GridPosition.Y);
}

// GetNeighbors in the 4 directions.
TArray<FGridCell*> UPathFinder::GetNeighbors(FGridCell* Cell)
{
    TArray<FGridCell*> Neighbors;

    FIntPoint Pos = Cell->GridPosition;

    FIntPoint Offsets[] = {
        FIntPoint(0, 1),   // Up
        FIntPoint(0, -1),  // Down
        FIntPoint(-1, 0),  // Left
        FIntPoint(1, 0)    // Right
    };

    for (const FIntPoint& Offset : Offsets)
    {
        FIntPoint NewPos = Pos + Offset;
        FGridCell* Neighbor = GetCellAtGrid(NewPos);
        if (Neighbor && Neighbor->bIsWalkable)
        {
            Neighbors.Add(Neighbor);
        }
    }

    return Neighbors;
}


bool UPathFinder::LoadGridFromFile(FString FilePath, FString CostConfigPath)
{
    FString FullPath = FPaths::ProjectContentDir() / FilePath;
    FString FileContent;

    // Load the file with the grid.
    if (!FFileHelper::LoadFileToString(FileContent, *FullPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Pathfinder: Could not load the file: %s"), *FullPath);
        return false;
    }

    // Load the file with the configuration of the Costs if it exists.
    if (!CostConfigPath.IsEmpty())
    {
        FString CostFullPath = FPaths::ProjectContentDir() / CostConfigPath;
        FString CostContent;
        if (FFileHelper::LoadFileToString(CostContent, *CostFullPath))
        {
            TArray<FString> Lines;
            CostContent.ParseIntoArrayLines(Lines);
            for (const FString& Line : Lines)
            {
                if (Line.Len() >= 3)
                {
                    char Key = Line[0];
                    float Value = FCString::Atof(*Line.Mid(2));
                    CostMap.Add(Key, Value);
                }
            }
            UE_LOG(LogTemp, Log, TEXT("Pathfinder: Loaded configuration of costs with %d entrances."), CostMap.Num());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Pathfinder: Could not load the configuration of costs, using default values."));
        }
    }

    // Parse the grid.
    TArray<FString> GridLines;
    FileContent.ParseIntoArrayLines(GridLines);

    GridSize.X = GridLines[0].Len();
    GridSize.Y = GridLines.Num();

    Grid.Empty();
    Grid.Reserve(GridSize.X * GridSize.Y);

    for (int32 Y = 0; Y < GridSize.Y; ++Y)
    {
        int32 InvertedY = GridSize.Y - 1 - Y;

        for (int32 X = 0; X < GridSize.X; ++X)
        {
            char Char = GridLines[InvertedY][X];
            float Cost = 1.f;
            bool bWalkable = true;

            // Set the properties based on the character.
            if (CostMap.Contains(Char))
            {
                Cost = CostMap[Char];
                bWalkable = (Cost > 0.f);
            }
            else if (Char == '#')
            {
                bWalkable = false;
                Cost = 0.f;
            }
            else
            {
                // If its a character without configuration, use the default one.
                Cost = static_cast<float>(Char - 'A' + 1);
                bWalkable = true;
            }

            // Calculate the position in the world.
            FVector WorldLoc = GridOrigin + FVector(X * CellSize, 0.f, Y * CellSize);

            // Create the cell.
            FGridCell Cell(FIntPoint(X, Y), WorldLoc, Cost, bWalkable);
            Grid.Add(Cell);
        }
    }

    int32 WalkableCount = 0;
    for (const FGridCell& Cell : Grid)
    {
        if (Cell.bIsWalkable) WalkableCount++;
    }

    UE_LOG(LogTemp, Log, TEXT("Pathfinder: Grid loaded - %dx%d, %d walkable cells."),
        GridSize.X, GridSize.Y, WalkableCount);
    return true;
}

void UPathFinder::SetupDefaultGrid(int32 GridSizeX, int32 GridSizeY, float InCellSize)
{
    CellSize = InCellSize;
    GridSize = FIntPoint(GridSizeX, GridSizeY);
    GridOrigin = FVector(-GridSizeX * CellSize / 2, 0, -GridSizeY * CellSize / 2);

    Grid.Empty();
    Grid.Reserve(GridSizeX * GridSizeY);

    for (int32 Y = 0; Y < GridSizeY; ++Y)
    {
        for (int32 X = 0; X < GridSizeX; ++X)
        {
            // Some obstacles for tests.
            bool bIsObstacle = (X == 3 && Y > 1 && Y < 8) || (X == 5 && Y == 5);

            FVector WorldLoc = GridOrigin + FVector(X * CellSize, 0.f, Y * CellSize);
            FGridCell Cell(FIntPoint(X, Y), WorldLoc, 1.f, !bIsObstacle);
            Grid.Add(Cell);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Pathfinder: Created default Grid - %dx%d."), GridSizeX, GridSizeY);
}

FGridCell* UPathFinder::GetCellAtLocation(FVector Location)
{
    FVector LocalPos = Location - GridOrigin;
    int32 X = FMath::RoundToInt(LocalPos.X / CellSize);
    int32 Y = FMath::RoundToInt(LocalPos.Z / CellSize); // I will use Z since Y is my vertical Axis.

    return GetCellAtGrid(FIntPoint(X, Y));
}

FGridCell* UPathFinder::GetCellAtGrid(FIntPoint GridPos)
{
    if (GridPos.X < 0 || GridPos.X >= GridSize.X ||
        GridPos.Y < 0 || GridPos.Y >= GridSize.Y)
    {
        return nullptr;
    }

    int32 Index = GridPos.Y * GridSize.X + GridPos.X;
    return &Grid[Index];
}

void UPathFinder::DrawGrid(bool bDrawCosts)
{
    if (!World) return;

    for (const FGridCell& Cell : Grid)
    {
        FVector Center = Cell.WorldLocation;
        
        // The thin axis is the Y since that is my vertical Axis.
        FVector Extents = FVector(CellSize / 2 - 5, 2.f, CellSize / 2 - 5);

        // Color based on the typee of cell.
        FColor Color;
        if (!Cell.bIsWalkable)
        {
            Color = FColor::Red;  // Obstacle
        }
        else if (Cell.Cost > 1.f)
        {
            // Areas with high cost: lerp from green to red.
            float Intensity = FMath::Clamp((Cell.Cost - 1.f) / 10.f, 0.f, 1.f);
            Color = FColor(
                FMath::Lerp(0, 255, Intensity),
                FMath::Lerp(255, 0, Intensity),
                0,
                255
            );
        }
        else
        {
            Color = FColor::Green;  // Normal cell
        }

        // If it's the start of end cell, then set their respective color.
        if (&Cell == CurrentStart)
        {
            Color = FColor::Blue;
        }
        else if (&Cell == CurrentEnd)
        {
            Color = FColor::Red;
        }

        DrawDebugBox(World, Center, Extents, Color, false, -1.f, 0, 4.f);

        // Draw cost if it's active.
        if (bDrawCosts && Cell.bIsWalkable && Cell.Cost != 1.f)
        {

            FString CostStr = FString::SanitizeFloat(Cell.Cost);
            DrawDebugString(World, Center + FVector(0, 15.f, 0), CostStr, nullptr, FColor::White, -1.f, false, 0.5f);
        }
    }
}

void UPathFinder::DrawPath(const TArray<FVector>& Path, FColor Color)
{
    if (!World || Path.Num() < 2) return;

    // Draw lines of the path.
    for (int32 i = 0; i < Path.Num() - 1; ++i)
    {
        FVector Start = Path[i];
        FVector End = Path[i + 1];
        Start.Y = 0.0f;
        End.Y = 0.0f;

        DrawDebugLine(World, Start, End, Color, false, -1.0f, 0, 5.0f);
    }

    // Draw waypoints
    for (const FVector& Point : Path)
    {
        DrawDebugSphere(World, Point, 15.f, 8, Color, false, -1.f, 0);
    }

    // Draw start and end with their specific colors.
    if (Path.Num() > 0)
    {
        FVector Start = Path[0];
        Start.Y = 0.f;
        DrawDebugSphere(World, Start, 25.0f, 12, FColor::Blue, false, -1.0f, 0);

        FVector End = Path.Last();
        End.Y = 0.f;
        DrawDebugSphere(World, End, 25.0f, 12, FColor::Red, false, -1.0f, 0);
    }
}