#include "debugdraw.h"

#include "Subsystems/EditorActorSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ArrowComponent.h"
#include "Math/Color.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProceduralMeshComponent.h"
#include "PaperSpriteActor.h"
#include "PaperSpriteComponent.h"
#include "CompGeom/PolygonTriangulation.h"
#include "IndexTypes.h"



UActorComponent* GetComponentByName(const AActor* Actor, FName ComponentName)
{
    if (Actor)
    {
        // Iterate through all components attached to the actor
        TArray<UActorComponent*> Components;
        Actor->GetComponents(Components);

        for (UActorComponent* Component : Components)
        {
            if (Component && Component->GetFName() == ComponentName)
            {
                // Component with the specified name found
                return Component;
            }
        }

        // Component with the specified name not found
        UE_LOG(LogTemp, Warning, TEXT("Component with name %s not found in actor: %s"), *ComponentName.ToString(), *Actor->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid actor provided"));
    }

    return nullptr;
}



AActor* GetActorByLabelInEditor(const AActor* owner, const FString& label)
{
    // Get a reference to the editor actor subsystem
    UEditorActorSubsystem* EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();

    if (EditorActorSubsystem)
    {
        // Use the editor actor subsystem
        // For example, you can spawn or manipulate actors
        auto all_actors = EditorActorSubsystem->GetAllLevelActors();

        for (AActor* Actor : all_actors)
        {
            // Check if the actor has a label
            FString ActorLabel = Actor->GetActorLabel();
            if (ActorLabel == label)
                return Actor;
        }
    }
    return NULL;
}

AActor* GetActorByLabel(const AActor* owner, const FString& label)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(owner, AActor::StaticClass(), FoundActors);

    // Iterate over the found actors
    for (AActor* Actor : FoundActors)
    {
        // Do something with the actor
        // Check if the actor has a label
        FString ActorLabel = Actor->GetActorLabel();
        if (ActorLabel == label)
            return Actor;
    }
    return NULL;
}

void SetCircle(const AActor* owner, const FString& circle_name, const FVector& position, float radius, FLinearColor circleColor)
{
    // Get Actor
    AActor* circle_actor = GetActorByLabel(owner, circle_name);
    if (!circle_actor)
        return;

    APaperSpriteActor* spriteActor = Cast< APaperSpriteActor>(circle_actor);
    if (!spriteActor)
        return;
    UPaperSpriteComponent* render_component = spriteActor->GetRenderComponent();
    if (!render_component)
        return;
    render_component->SetSpriteColor(circleColor);

    // Set position
    circle_actor->SetActorLocation(position);


    // Calculate scale from radius
    float newscale = radius / 50.0f;
    circle_actor->SetActorScale3D(FVector(newscale, newscale, newscale));
}

void SetArrow(const AActor* owner, const FString& arrow_name, const FVector& direction, float length)
{
    // Get Component
    UActorComponent* ActorComponent = GetComponentByName(owner, FName(arrow_name));
    UArrowComponent* ArrowComponent = Cast<UArrowComponent>(ActorComponent);

    if (ArrowComponent)
    {

        // Calculate angle from direction

        // Set rotation
        FVector arrow_dir = direction;
        arrow_dir.Normalize();
        float angle = atan2(arrow_dir.Z, arrow_dir.X);
        float deg = FMath::RadiansToDegrees(angle);

        FRotator rot(deg, 0.0f, 0.0f);
        ArrowComponent->SetRelativeRotation(rot);

        // Set Length
        ArrowComponent->ArrowLength = length;

        ArrowComponent->SetArrowColor(ArrowComponent->ArrowColor); // Force Update
    }
}

void SetPath(const AActor* owner, const FString& path_actor_name, const FString& path_name, const TArray<FVector>& PathPoints, float PathThickness, UMaterialInterface* material)
{
    // Check if we have enough points to create the path
    if (PathPoints.Num() < 2)
    {
        UE_LOG(LogTemp, Error, TEXT("Path must contain at least two points"));
        return;
    }

    AActor* path_actor = GetActorByLabel(owner, path_actor_name);
    if (!path_actor)
        return;
    // Get Component
    UActorComponent* ActorComponent = GetComponentByName(path_actor, FName(path_name));
    UProceduralMeshComponent* ProceduralMeshComponent = Cast<UProceduralMeshComponent>(ActorComponent);

    if (ProceduralMeshComponent)
    {
        // Arrays to hold mesh data
        TArray<FVector> Vertices;
        TArray<int32> Triangles;
        TArray<FVector> Normals;
        TArray<FVector2D> UVs;

        // Generate vertices, triangles, normals, and UVs
        for (int32 i = 0; i < PathPoints.Num(); ++i)
        {
            // Get current and next points
            FVector CurrentPoint = PathPoints[i];
            FVector NextPoint = PathPoints[(i + 1) % PathPoints.Num()];

            // Calculate direction vector and perpendicular vector
            FVector Direction = (NextPoint - CurrentPoint).GetSafeNormal();
            FVector Perpendicular = FVector::CrossProduct(Direction, FVector::YAxisVector).GetSafeNormal();

            // Calculate vertices for outer edge of path
            FVector OuterTop = CurrentPoint + (Perpendicular * (PathThickness / 2));
            FVector OuterBottom = CurrentPoint - (Perpendicular * (PathThickness / 2));

            // Calculate vertices for inner edge of path
            FVector InnerTop = NextPoint + (Perpendicular * (PathThickness / 2));
            FVector InnerBottom = NextPoint - (Perpendicular * (PathThickness / 2));

            // Add vertices
            Vertices.Add(OuterTop);
            Vertices.Add(OuterBottom);
            Vertices.Add(InnerBottom);
            Vertices.Add(InnerTop);

            // Calculate UVs and normals (for simplicity, UVs and normals are calculated per vertex)
            for (int32 j = 0; j < 4; ++j)
            {
                UVs.Add(FVector2D((float)j / 4, 0)); // Linear UV mapping along path
                Normals.Add(Direction); // Use the same normal for all vertices
            }

            // Add triangles
            int32 StartIndex = i * 4;
            Triangles.Add(StartIndex);
            Triangles.Add(StartIndex + 1);
            Triangles.Add(StartIndex + 2);
            Triangles.Add(StartIndex);
            Triangles.Add(StartIndex + 2);
            Triangles.Add(StartIndex + 3);
        }

        // Create mesh section
        ProceduralMeshComponent->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
        ProceduralMeshComponent->SetMaterial(0, material);
    }
}

void SetPolygons(const AActor* owner, const FString& polygon_actor_name, const FString& polygon_name, const TArray<TArray<FVector> >& polygons, UMaterialInterface* material)
{
    AActor* polygon_actor = GetActorByLabel(owner, polygon_actor_name);
    if (!polygon_actor)
        return;
    // Get Component
    UActorComponent* ActorComponent = GetComponentByName(polygon_actor, FName(polygon_name));
    UProceduralMeshComponent* ProceduralMeshComponent = Cast<UProceduralMeshComponent>(ActorComponent);

    if (ProceduralMeshComponent)
    {
        // Combine vertices, UVs, and normals for all polygons
        TArray<FVector> CombinedVertices;
        TArray<FVector> CombinedNormals;
        TArray<FVector2D> CombinedUVs;
        TArray<int32> CombinedTriangles;

        int32 VertexOffset = 0;
        for (const TArray<FVector>& Polygon : polygons)
        {
            // Triangulate the polygon
            TArray<int32> PolygonTriangles;
            TArray< UE::Geometry::FIndex3i > OutTriangles;
            PolygonTriangulation::TriangulateSimplePolygon(Polygon, OutTriangles);

            // Add vertices, normals, and UVs for the current polygon
            for (const FVector& Vertex : Polygon)
            {
                CombinedVertices.Add(Vertex);
                CombinedNormals.Add(FVector(0.0f, 1.0f, 0.0f));
            }

            // Add triangles for the current polygon, adjusting vertex indices
            for (UE::Geometry::FIndex3i Triangle : OutTriangles)
            {
                CombinedTriangles.Add(VertexOffset + Triangle.A);
                CombinedTriangles.Add(VertexOffset + Triangle.C);
                CombinedTriangles.Add(VertexOffset + Triangle.B);
            }

            // Update vertex offset for the next polygon
            VertexOffset += Polygon.Num();
        }

        ProceduralMeshComponent->CreateMeshSection(0, CombinedVertices, CombinedTriangles, CombinedNormals, CombinedUVs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
        ProceduralMeshComponent->SetMaterial(0, material);
    }
}