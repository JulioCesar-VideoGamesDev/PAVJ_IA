#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../navmesh/navMesh.h"
#include "PathFinder_NavMesh.generated.h"

/**
 * Clase principal para Pathfinding sobre NavMesh
 */
UCLASS(BlueprintType)
class MPV_PRACTICAS_API UPathFinder_NavMesh : public UObject
{
    GENERATED_BODY()

public:
    UPathFinder_NavMesh();

    // ============================================
    // FUNCIONES PRINCIPALES
    // ============================================

    /**
     * Encuentra un camino entre dos posiciones en el NavMesh
     * @param StartLocation - Posición de inicio en el mundo 3D
     * @param EndLocation - Posición de destino en el mundo 3D
     * @return TArray<FVector> - Lista de waypoints del camino
     */
    UFUNCTION(BlueprintCallable, Category = "NavMesh")
    TArray<FVector> FindPath(FVector StartLocation, FVector EndLocation);

    /**
     * Carga un NavMesh desde un archivo XML
     * @param FilePath - Ruta del archivo XML
     * @return bool - true si se cargó correctamente
     */
    UFUNCTION(BlueprintCallable, Category = "NavMesh")
    bool LoadNavMeshFromFile(FString FilePath);

    TArray<TArray<FVector>> GetNavMeshPolygons();

    UFUNCTION(BlueprintCallable, Category = "NavMesh")
    TArray<FVector> PolygonToFVectorArray(const FNavPolygon& Polygon, float Height) const;

    /**
     * Dibuja un camino en DebugDraw
     */
    UFUNCTION(BlueprintCallable, Category = "NavMesh")
    void DrawPath(const TArray<FVector>& Path, FColor Color = FColor::Yellow);

    /**
     * Obtiene el último camino calculado
     */
    UFUNCTION(BlueprintCallable, Category = "NavMesh")
    TArray<FVector> GetLastPath() const { return LastPath; }

    /**
     * Obtiene el polígono que contiene un punto
     */
    UFUNCTION(BlueprintCallable, Category = "NavMesh")
    int32 GetPolygonAtLocation(FVector Location) const;

    /**
     * Obtiene la posición en el mundo 3D de un punto 2D
     */
    UFUNCTION(BlueprintCallable, Category = "NavMesh")
    FVector GetWorldPosition(const FNavPoint2D& Point, float Z = 0.0f) const;

    // Configuración
    float HeightOffset = 0.0f;  // Altura del NavMesh (Y en Unreal)

    // Necesario para dibujar
    class UWorld* World = nullptr;

protected:
    // ============================================
    // ALGORITMO A* SOBRE NAVMESH
    // ============================================

    /**
     * Implementación de A* para NavMesh
     */
    TArray<FNavPoint2D> AStarOnNavMesh(int32 StartPolygon, int32 EndPolygon,
        const FNavPoint2D& StartPos,
        const FNavPoint2D& EndPos);

    /**
     * Reconstruye el camino desde los nodos
     */
    TArray<FNavPoint2D> ReconstructPath(FNavMeshNode* EndNode);

    /**
     * Heurística: Distancia entre dos puntos
     */
    float Heuristic(const FNavPoint2D& A, const FNavPoint2D& B) const;

    /**
     * Obtiene los vecinos de un polígono (polígonos conectados por links)
     */
    TArray<int32> GetNeighbors(int32 PolygonIndex) const;

    /**
     * Obtiene el centro de una arista compartida entre dos polígonos
     */
    FNavPoint2D GetSharedEdgeCenter(int32 PolyA, int32 PolyB) const;

    /**
     * Encuentra el punto de entrada a un polígono desde otro
     */
    FNavPoint2D GetEntryPoint(int32 FromPolygon, int32 ToPolygon, const FNavPoint2D& Position) const;

    //TArray<FNavPolygon> GetPolygons() const { return Polygons; }

private:
    // ============================================
    // DATOS DEL NAVMESH
    // ============================================

    UPROPERTY()
    TArray<FNavPolygon> Polygons;

    UPROPERTY()
    TArray<FNavLink> Links;

    // Mapa de adyacencia: polígono > lista de polígonos vecinos
    TMap<int32, TArray<int32>> AdjacencyMap;

    // Último camino calculado
    TArray<FVector> LastPath;

    // Posiciones de inicio y fin (para debug)
    FNavPoint2D StartPoint2D;
    FNavPoint2D EndPoint2D;
    int32 StartPolygonIndex = -1;
    int32 EndPolygonIndex = -1;

    // ============================================
    // FUNCIONES DE PARSEO XML
    // ============================================

    bool ParseXML(const FString& XMLContent);
    bool ParsePolygons(const FString& XMLContent);
    bool ParseLinks(const FString& XMLContent);
    void BuildAdjacencyMap();
};