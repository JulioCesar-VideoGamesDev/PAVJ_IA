#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "navMesh.generated.h"

USTRUCT(BlueprintType)
struct FNavPoint2D
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float X = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float Y = 0.0f;

    FNavPoint2D() {}

    FNavPoint2D(float InX, float InY)
        : X(InX), Y(InY) {
    }

    FVector ToFVector(float Z = 0.0f) const
    {
        return FVector(X, Z, Y);
    }
};

USTRUCT(BlueprintType)
struct FNavEdge
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FNavPoint2D Start;

    UPROPERTY(BlueprintReadWrite)
    FNavPoint2D End;

    UPROPERTY(BlueprintReadWrite)
    int32 PolygonIndex = -1;

    UPROPERTY(BlueprintReadWrite)
    int32 EdgeIndex = -1;

    FNavEdge() {}

    FNavEdge(const FNavPoint2D& InStart, const FNavPoint2D& InEnd)
        : Start(InStart), End(InEnd) {
    }

    FNavPoint2D GetCenter2D() const
    {
        return FNavPoint2D(
            (Start.X + End.X) * 0.5f,
            (Start.Y + End.Y) * 0.5f
        );
    }

    FVector GetCenter3D(float Z = 0.0f) const
    {
        return FVector(
            (Start.X + End.X) * 0.5f,
            Z,
            (Start.Y + End.Y) * 0.5f
        );
    }

    FVector GetCenter(float Z = 0.0f) const
    {
        return GetCenter3D(Z);
    }

    float GetLength() const
    {
        return FMath::Sqrt(FMath::Square(End.X - Start.X) + FMath::Square(End.Y - Start.Y));
    }
};

USTRUCT(BlueprintType)
struct FNavPolygon
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TArray<FNavPoint2D> Points;

    UPROPERTY(BlueprintReadWrite)
    int32 PolygonIndex = -1;

    TArray<FNavEdge> Edges;

    TArray<int32> NeighborIndices;

    FNavPoint2D Center;

    FNavPolygon() {}

    void BuildEdges()
    {
        Edges.Empty();
        for (int32 i = 0; i < Points.Num(); i++)
        {
            int32 NextIndex = (i + 1) % Points.Num();
            FNavEdge Edge(Points[i], Points[NextIndex]);
            Edge.PolygonIndex = PolygonIndex;
            Edge.EdgeIndex = i;
            Edges.Add(Edge);
        }

        // Calculate center
        Center = FNavPoint2D(0.0f, 0.0f);
        for (const FNavPoint2D& Point : Points)
        {
            Center.X += Point.X;
            Center.Y += Point.Y;
        }
        Center.X /= Points.Num();
        Center.Y /= Points.Num();
    }

    bool IsPointInside(const FNavPoint2D& Point) const
    {
        bool bInside = false;
        int32 j = Points.Num() - 1;

        for (int32 i = 0; i < Points.Num(); i++)
        {
            if (((Points[i].Y > Point.Y) != (Points[j].Y > Point.Y)) &&
                (Point.X < (Points[j].X - Points[i].X) * (Point.Y - Points[i].Y) / (Points[j].Y - Points[i].Y) + Points[i].X))
            {
                bInside = !bInside;
            }
            j = i;
        }
        return bInside;
    }

    FNavPoint2D GetClosestPointOnEdge(const FNavPoint2D& Point, int32 EdgeIndex) const
    {
        if (EdgeIndex < 0 || EdgeIndex >= Edges.Num())
            return Point;

        const FNavEdge& Edge = Edges[EdgeIndex];

        FVector2D A(Edge.Start.X, Edge.Start.Y);
        FVector2D B(Edge.End.X, Edge.End.Y);
        FVector2D P(Point.X, Point.Y);

        FVector2D AB = B - A;
        FVector2D AP = P - A;

        float t = FVector2D::DotProduct(AP, AB) / FVector2D::DotProduct(AB, AB);
        t = FMath::Clamp(t, 0.0f, 1.0f);

        FVector2D Closest = A + t * AB;
        return FNavPoint2D(Closest.X, Closest.Y);
    }
};

USTRUCT(BlueprintType)
struct FNavLink
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 StartPolygon = -1;

    UPROPERTY(BlueprintReadWrite)
    int32 StartEdgeStart = -1;

    UPROPERTY(BlueprintReadWrite)
    int32 StartEdgeEnd = -1;

    UPROPERTY(BlueprintReadWrite)
    int32 EndPolygon = -1;

    UPROPERTY(BlueprintReadWrite)
    int32 EndEdgeStart = -1;

    UPROPERTY(BlueprintReadWrite)
    int32 EndEdgeEnd = -1;

    FNavEdge SharedEdge;

    FNavLink() {}

    void CalculateSharedEdge(const TArray<FNavPolygon>& Polygons)
    {
        if (StartPolygon < 0 || EndPolygon < 0 ||
            StartPolygon >= Polygons.Num() || EndPolygon >= Polygons.Num())
            return;

        const FNavPolygon& StartPoly = Polygons[StartPolygon];
        const FNavPolygon& EndPoly = Polygons[EndPolygon];

        FNavPoint2D StartPoint = StartPoly.Points[StartEdgeStart];
        FNavPoint2D EndPoint = StartPoly.Points[StartEdgeEnd];

        SharedEdge = FNavEdge(StartPoint, EndPoint);
        SharedEdge.PolygonIndex = StartPolygon;
    }
};

struct FNavMeshNode
{
    int32 PolygonIndex = -1;
    FNavPoint2D Position;

    float GCost = 0.0f;
    float HCost = 0.0f;
    float FCost = 0.0f;

    FNavMeshNode* Parent = nullptr;
    bool bIsInOpenSet = false;
    bool bIsInClosedSet = false;

    FNavMeshNode() {}

    FNavMeshNode(int32 InPolygonIndex, const FNavPoint2D& InPosition)
        : PolygonIndex(InPolygonIndex)
        , Position(InPosition)
        , GCost(0.0f)
        , HCost(0.0f)
        , FCost(0.0f)
        , Parent(nullptr)
        , bIsInOpenSet(false)
        , bIsInClosedSet(false)
    {
    }
};