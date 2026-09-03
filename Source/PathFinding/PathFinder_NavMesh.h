#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../navmesh/navMesh.h"
#include "PathFinder_NavMesh.generated.h"

UCLASS(BlueprintType)
class MPV_PRACTICAS_API UPathFinder_NavMesh : public UObject
{
    GENERATED_BODY()

public:
    UPathFinder_NavMesh();

    UFUNCTION(BlueprintCallable, Category = "NavMesh")
    TArray<FVector> FindPath(FVector StartLocation, FVector EndLocation);

    UFUNCTION(BlueprintCallable, Category = "NavMesh")
    bool LoadNavMeshFromFile(FString FilePath);

    TArray<TArray<FVector>> GetNavMeshPolygons();

    UFUNCTION(BlueprintCallable, Category = "NavMesh")
    TArray<FVector> PolygonToFVectorArray(const FNavPolygon& Polygon, float Height) const;

    UFUNCTION(BlueprintCallable, Category = "NavMesh")
    void DrawPath(const TArray<FVector>& Path, FColor Color = FColor::Yellow);

    UFUNCTION(BlueprintCallable, Category = "NavMesh")
    TArray<FVector> GetLastPath() const { return LastPath; }

    UFUNCTION(BlueprintCallable, Category = "NavMesh")
    int32 GetPolygonAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "NavMesh")
    FVector GetWorldPosition(const FNavPoint2D& Point, float Z = 0.0f) const;


    float HeightOffset = 0.0f;  // Height offset relative to the ground.

    class UWorld* World = nullptr;

protected:
    // A* algorithim
    TArray<FNavPoint2D> AStarOnNavMesh(int32 StartPolygon, int32 EndPolygon,
        const FNavPoint2D& StartPos,
        const FNavPoint2D& EndPos);

    TArray<FNavPoint2D> ReconstructPath(FNavMeshNode* EndNode);

    // Calculates the Heuristic
    float Heuristic(const FNavPoint2D& A, const FNavPoint2D& B) const;

    
    TArray<int32> GetNeighbors(int32 PolygonIndex) const;

    // Returns the center of the shared edge between two polygons.
    FNavPoint2D GetSharedEdgeCenter(int32 PolyA, int32 PolyB) const;

    // Returns the Link point between two polygons.
    FNavPoint2D GetEntryPoint(int32 FromPolygon, int32 ToPolygon, const FNavPoint2D& Position) const;

private:

    UPROPERTY()
    TArray<FNavPolygon> Polygons;

    UPROPERTY()
    TArray<FNavLink> Links;

    TMap<int32, TArray<int32>> AdjacencyMap;

    TArray<FVector> LastPath;

    FNavPoint2D StartPoint2D;
    FNavPoint2D EndPoint2D;
    int32 StartPolygonIndex = -1;
    int32 EndPolygonIndex = -1;

    bool ParseXML(const FString& XMLContent);
    /*bool ParsePolygons(const FString& XMLContent);
    bool ParseLinks(const FString& XMLContent);*/
    void BuildAdjacencyMap();
};