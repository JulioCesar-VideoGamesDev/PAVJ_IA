#pragma once

#include "CoreMinimal.h"
#include "Steering.h"
#include "../obstacles/obstacles.h"
#include "ObstacleAvoidanceSteering.generated.h"

class AAICharacter;

USTRUCT()
struct FObstacleCollisionResult
{
    GENERATED_BODY()

    bool bWillCollide{ false };

    int32 ObstacleIndex{ INDEX_NONE };

    FVector ClosestPoint{ FVector::ZeroVector };
    FVector Difference{ FVector::ZeroVector };

    float Distance{ 0.f };
};

UCLASS()
class MPV_PRACTICAS_API UObstacleAvoidanceSteering : public UObject, public ISteering
{
    GENERATED_BODY()

public:

    // Pointer the the AICharacter used to GetSteering.
    UPROPERTY()
    TObjectPtr<AAICharacter> AICharacter;

    TArray<FObstacleAttr> ObstaclesArray;

    UFUNCTION(BlueprintCallable, Category = "Streering")
    virtual void GetSteering(FSteeringOutput& SteeringOutput) override;

    FObstacleCollisionResult FindCollision(
        const FSteeringOutput& InputSteering);
    
    UFUNCTION(BlueprintCallable, Category = "Streering")
    void FindCollisionAndGetSteering(
        const FSteeringOutput& InputSteering,
        FSteeringOutput& SteeringOutput);

    UPROPERTY(EditDefaultsOnly, Category = "Obstacle Avoidance")
    float AvoidanceStrength{ 10.f };

    // Rturns if the AI will collide. -_-
    UFUNCTION(BlueprintCallable, Category = "ObstacleDetection")
    bool WillCollide(FSteeringOutput InputSteering);

    bool DoDrawDebug{ false };

    FObstacleCollisionResult GetFoundedCollision() const { return FoundedCollision; }

private:

    FObstacleCollisionResult FoundedCollision;

    UFUNCTION()
    void DrawDebug();
};