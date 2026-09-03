#include "PathFinder_NavMesh.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "XmlParser.h"

UPathFinder_NavMesh::UPathFinder_NavMesh()
{
}

bool UPathFinder_NavMesh::LoadNavMeshFromFile(FString FilePath)
{
    FString FullPath = FPaths::ProjectContentDir() / FilePath;
    FString XMLContent;

    if (!FFileHelper::LoadFileToString(XMLContent, *FullPath))
    {
        UE_LOG(LogTemp, Error, TEXT("NavMesh: Could not load the file: %s"), *FullPath);
        return false;
    }

    Polygons.Empty();
    Links.Empty();
    AdjacencyMap.Empty();

    if (!ParseXML(XMLContent))
    {
        UE_LOG(LogTemp, Error, TEXT("NavMesh: Error while parsing the XML file"));
        return false;
    }

    BuildAdjacencyMap();

    UE_LOG(LogTemp, Log, TEXT("NavMesh: Loaded correctly - %d poligons, %d links"),
        Polygons.Num(), Links.Num());

    return true;
}

bool UPathFinder_NavMesh::ParseXML(const FString& XMLContent)
{
    // XML parser from Unreal.
    FXmlFile XmlFile;

    if (!XmlFile.LoadFile(XMLContent, EConstructMethod::ConstructFromBuffer))
    {
        UE_LOG(LogTemp, Error, TEXT("NavMesh: Error while loading the XML"));
        return false;
    }

    FXmlNode* RootNode = XmlFile.GetRootNode();
    if (!RootNode || RootNode->GetTag() != TEXT("navmesh"))
    {
        UE_LOG(LogTemp, Error, TEXT("NavMesh: Root node isn't 'navmesh'"));
        return false;
    }

    // Parsear polígonos
    FXmlNode* PolygonsNode = RootNode->FindChildNode(TEXT("polygons"));
    if (!PolygonsNode)
    {
        UE_LOG(LogTemp, Error, TEXT("NavMesh: Couldn't find node 'polygons'"));
        return false;
    }

    TArray<FXmlNode*> PolygonNodes = PolygonsNode->GetChildrenNodes();
    for (FXmlNode* PolyNode : PolygonNodes)
    {
        if (PolyNode->GetTag() != TEXT("polygon"))
            continue;

        FNavPolygon NewPolygon;
        NewPolygon.PolygonIndex = Polygons.Num();

        TArray<FXmlNode*> PointNodes = PolyNode->GetChildrenNodes();
        for (FXmlNode* PointNode : PointNodes)
        {
            if (PointNode->GetTag() != TEXT("point"))
                continue;

            FNavPoint2D Point;
            Point.X = FCString::Atof(*PointNode->GetAttribute(TEXT("x")));
            Point.Y = FCString::Atof(*PointNode->GetAttribute(TEXT("y")));
            Point.Y *=  -1;
            NewPolygon.Points.Add(Point);
        }

        NewPolygon.BuildEdges();
        Polygons.Add(NewPolygon);
    }

    // Parse Links
    FXmlNode* LinksNode = RootNode->FindChildNode(TEXT("links"));
    if (LinksNode)
    {
        TArray<FXmlNode*> LinkNodes = LinksNode->GetChildrenNodes();
        for (FXmlNode* LinkNode : LinkNodes)
        {
            if (LinkNode->GetTag() != TEXT("link"))
                continue;

            FNavLink NewLink;

            FXmlNode* StartNode = LinkNode->FindChildNode(TEXT("start"));
            if (StartNode)
            {
                NewLink.StartPolygon = FCString::Atoi(*StartNode->GetAttribute(TEXT("polygon")));
                NewLink.StartEdgeStart = FCString::Atoi(*StartNode->GetAttribute(TEXT("edgestart")));
                NewLink.StartEdgeEnd = FCString::Atoi(*StartNode->GetAttribute(TEXT("edgeend")));
            }

            FXmlNode* EndNode = LinkNode->FindChildNode(TEXT("end"));
            if (EndNode)
            {
                NewLink.EndPolygon = FCString::Atoi(*EndNode->GetAttribute(TEXT("polygon")));
                NewLink.EndEdgeStart = FCString::Atoi(*EndNode->GetAttribute(TEXT("edgestart")));
                NewLink.EndEdgeEnd = FCString::Atoi(*EndNode->GetAttribute(TEXT("edgeend")));
            }

            NewLink.CalculateSharedEdge(Polygons);
            Links.Add(NewLink);
        }
    }

    return true;
}

void UPathFinder_NavMesh::BuildAdjacencyMap()
{
    AdjacencyMap.Empty();

    for (int32 i = 0; i < Polygons.Num(); i++)
    {
        AdjacencyMap.Add(i, TArray<int32>());
    }

    for (const FNavLink& Link : Links)
    {
        if (Link.StartPolygon >= 0 && Link.StartPolygon < Polygons.Num())
        {
            AdjacencyMap[Link.StartPolygon].AddUnique(Link.EndPolygon);
        }
        if (Link.EndPolygon >= 0 && Link.EndPolygon < Polygons.Num())
        {
            AdjacencyMap[Link.EndPolygon].AddUnique(Link.StartPolygon);
        }
    }
}

TArray<FVector> UPathFinder_NavMesh::FindPath(FVector StartLocation, FVector EndLocation)
{
    LastPath.Empty();

    // Convert to 2D coordinates from the NavMesh
    StartPoint2D = FNavPoint2D(StartLocation.X, StartLocation.Z);
    EndPoint2D = FNavPoint2D(EndLocation.X, EndLocation.Z);

    // Find polygons based on the positions.
    StartPolygonIndex = GetPolygonAtLocation(StartLocation);
    EndPolygonIndex = GetPolygonAtLocation(EndLocation);

    if (StartPolygonIndex < 0 || EndPolygonIndex < 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("NavMesh: Start or End out of navmesh"));
        return LastPath;
    }

    if (StartPolygonIndex == EndPolygonIndex)
    {
        // Same polygon so it's a straight line.
        LastPath.Add(StartLocation);
        LastPath.Add(EndLocation);
        return LastPath;
    }

    // Execute A* with the polygons
    TArray<FNavPoint2D> PathPoints2D = AStarOnNavMesh(StartPolygonIndex, EndPolygonIndex,
        StartPoint2D, EndPoint2D);

    if (PathPoints2D.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("NavMesh: couldn't find a path"));
        return LastPath;
    }

    for (const FNavPoint2D& Point2D : PathPoints2D)
    {
        LastPath.Add(FVector(Point2D.X, HeightOffset, Point2D.Y));
    }

    UE_LOG(LogTemp, Log, TEXT("NavMesh: Path found with %d puntos"), LastPath.Num());
    return LastPath;
}

TArray<FNavPoint2D> UPathFinder_NavMesh::AStarOnNavMesh(int32 StartPolygon, int32 EndPolygon,
    const FNavPoint2D& StartPos,
    const FNavPoint2D& EndPos)
{
    TArray<FNavMeshNode*> OpenSet;
    TArray<FNavMeshNode*> ClosedSet;
    TArray<FNavMeshNode> NodePool;

    // Initialize nodes.
    for (int32 i = 0; i < Polygons.Num(); i++)
    {
        FNavMeshNode Node(i, Polygons[i].Center);
        Node.GCost = INFINITY;
        Node.HCost = INFINITY;
        Node.FCost = INFINITY;
        Node.Parent = nullptr;
        Node.bIsInOpenSet = false;
        Node.bIsInClosedSet = false;
        NodePool.Add(Node);
    }

    // Initial Node
    FNavMeshNode* StartNode = &NodePool[StartPolygon];
    StartNode->Position = StartPos;
    StartNode->GCost = 0.0f;
    StartNode->HCost = Heuristic(StartPos, EndPos);
    StartNode->FCost = StartNode->GCost + StartNode->HCost;
    StartNode->bIsInOpenSet = true;
    OpenSet.Add(StartNode);

    // Last Node
    FNavMeshNode* EndNode = &NodePool[EndPolygon];
    EndNode->Position = EndPos;

    while (!OpenSet.IsEmpty())
    {
        // Find the node with less FCost.
        FNavMeshNode* Current = OpenSet[0];
        int32 CurrentIndex = 0;
        for (int32 i = 1; i < OpenSet.Num(); i++)
        {
            if (OpenSet[i]->FCost < Current->FCost)
            {
                Current = OpenSet[i];
                CurrentIndex = i;
            }
        }

        // If we arrive at the Polygon target.
        if (Current->PolygonIndex == EndPolygon)
        {
            TArray<FNavPoint2D> Path = ReconstructPath(Current);
            // Set the exact end point.
            Path.Add(EndPos);
            return Path;
        }

        // Swap from open to closed
        OpenSet.RemoveAt(CurrentIndex);
        Current->bIsInOpenSet = false;
        Current->bIsInClosedSet = true;
        ClosedSet.Add(Current);

        // Explore Neighbors
        TArray<int32> Neighbors = GetNeighbors(Current->PolygonIndex);
        for (int32 NeighborIndex : Neighbors)
        {
            FNavMeshNode* Neighbor = &NodePool[NeighborIndex];

            if (Neighbor->bIsInClosedSet)
                continue;

            // Calculate the entry point to the Neighbor.
            FNavPoint2D EntryPoint = GetEntryPoint(Current->PolygonIndex, NeighborIndex, Current->Position);

            float MoveCost = Heuristic(Current->Position, EntryPoint);
            float TentativeG = Current->GCost + MoveCost;

            if (!Neighbor->bIsInOpenSet)
            {
                Neighbor->bIsInOpenSet = true;
                OpenSet.Add(Neighbor);
            }
            else if (TentativeG >= Neighbor->GCost)
            {
                continue;
            }

            // Update the best path.
            Neighbor->Parent = Current;
            Neighbor->Position = EntryPoint;
            Neighbor->GCost = TentativeG;
            Neighbor->HCost = Heuristic(EntryPoint, EndPos);
            Neighbor->FCost = Neighbor->GCost + Neighbor->HCost;
        }
    }

    return TArray<FNavPoint2D>();
}

TArray<FNavPoint2D> UPathFinder_NavMesh::ReconstructPath(FNavMeshNode* EndNode)
{
    TArray<FNavPoint2D> Path;
    FNavMeshNode* Current = EndNode;

    while (Current)
    {
        Path.Add(Current->Position);
        Current = Current->Parent;
    }
    
    for (int32 i = 0; i < Path.Num() / 2; i++)
    {
        int32 j = Path.Num() - 1 - i;
        FNavPoint2D Temp = Path[i];
        Path[i] = Path[j];
        Path[j] = Temp;
    }

    return Path;
}

TArray<int32> UPathFinder_NavMesh::GetNeighbors(int32 PolygonIndex) const
{
    if (AdjacencyMap.Contains(PolygonIndex))
    {
        return AdjacencyMap[PolygonIndex];
    }
    return TArray<int32>();
}

FNavPoint2D UPathFinder_NavMesh::GetSharedEdgeCenter(int32 PolyA, int32 PolyB) const
{
    for (const FNavLink& Link : Links)
    {
        if ((Link.StartPolygon == PolyA && Link.EndPolygon == PolyB) ||
            (Link.StartPolygon == PolyB && Link.EndPolygon == PolyA))
        {
            // Calculate the center of the shared edge.
            FNavPoint2D Center = Link.SharedEdge.Start;
            Center.X = (Center.X + Link.SharedEdge.End.X) * 0.5f;
            Center.Y = (Center.Y + Link.SharedEdge.End.Y) * 0.5f;
            return Center;
        }
    }
    return FNavPoint2D();
}

FNavPoint2D UPathFinder_NavMesh::GetEntryPoint(int32 FromPolygon, int32 ToPolygon, const FNavPoint2D& Position) const
{
    // Get the center of the shared edge.
    FNavPoint2D EdgeCenter = GetSharedEdgeCenter(FromPolygon, ToPolygon);

    // If we couldn't find the edge then use the center of the next Polygon.
    if (EdgeCenter.X == 0.0f && EdgeCenter.Y == 0.0f)
    {
        if (ToPolygon >= 0 && ToPolygon < Polygons.Num())
        {
            return Polygons[ToPolygon].Center;
        }
        return Position;
    }

    return EdgeCenter;
}

float UPathFinder_NavMesh::Heuristic(const FNavPoint2D& A, const FNavPoint2D& B) const
{
    return FMath::Sqrt(FMath::Square(B.X - A.X) + FMath::Square(B.Y - A.Y));
}

int32 UPathFinder_NavMesh::GetPolygonAtLocation(FVector Location) const
{
    FNavPoint2D Point2D(Location.X, Location.Z);

    for (int32 i = 0; i < Polygons.Num(); i++)
    {
        if (Polygons[i].IsPointInside(Point2D))
        {
            return i;
        }
    }
    return -1;
}

FVector UPathFinder_NavMesh::GetWorldPosition(const FNavPoint2D& Point, float Z) const
{
    return FVector(Point.X, Z, Point.Y);
}

TArray<TArray<FVector>> UPathFinder_NavMesh::GetNavMeshPolygons()
{
    TArray<TArray<FVector>> Result;

    for (const FNavPolygon& Polygon : Polygons)
    {
        TArray<FVector> PolygonPoints = PolygonToFVectorArray(Polygon, HeightOffset);
        Result.Add(PolygonPoints);
    }

    return Result;
}

TArray<FVector> UPathFinder_NavMesh::PolygonToFVectorArray(const FNavPolygon& Polygon, float Height) const
{
    TArray<FVector> Result;

    for (const FNavPoint2D& Point : Polygon.Points)
    {
        Result.Add(GetWorldPosition(Point, Height));
    }

    return Result;
}

void UPathFinder_NavMesh::DrawPath(const TArray<FVector>& Path, FColor Color)
{
    if (!World || Path.Num() < 2) return;

    for (int32 i = 0; i < Path.Num() - 1; i++)
    {
        FVector Start = Path[i];
        FVector End = Path[i + 1];
        Start.Y = HeightOffset + 15.0f;
        End.Y = HeightOffset + 15.0f;

        DrawDebugLine(World, Start, End, Color, false, -1.0f, 0, 5.0f);
    }

    for (const FVector& Point : Path)
    {
        FVector Pos = Point;
        Pos.Y = HeightOffset + 15.0f;
        DrawDebugSphere(World, Pos, 8.0f, 8, Color, false, -1.0f, 0);
    }

    if (Path.Num() > 0)
    {
        FVector Start = Path[0];
        Start.Y = HeightOffset + 25.0f;
        DrawDebugSphere(World, Start, 20.0f, 12, FColor::Green, false, -1.0f, 0);

        FVector End = Path.Last();
        End.Y = HeightOffset + 25.0f;
        DrawDebugSphere(World, End, 20.0f, 12, FColor::Red, false, -1.0f, 0);
    }
}