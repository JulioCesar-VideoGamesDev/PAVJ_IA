
#include <CoreMinimal.h>

void SetArrow(const AActor* owner, const FString& arrow_name, const FVector& direction, float length);

void SetCircle(const AActor* owner, const FString& circle_name, const FVector& position, float radius, FLinearColor circleColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f));

void SetPath(const AActor* owner, const FString& path_actor_name, const FString& component_name, const TArray<FVector>& PathPoints, float PathThickness, UMaterialInterface* material);

void SetPolygons(const AActor* owner, const FString& polygon_actor_name, const FString& polygon_name, const TArray<TArray<FVector> >& polygons, UMaterialInterface* material);;