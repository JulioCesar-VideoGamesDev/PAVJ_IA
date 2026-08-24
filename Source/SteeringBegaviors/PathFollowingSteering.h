#pragma once

#include "CoreMinimal.h"
#include "Steering.h"
#include "PathFollowingSteering.generated.h"

class AAICharacter;
class UArriveSteering;
class UObstacleAvoidanceSteering;

struct FClosestPointResult
{
    FVector Point{ FVector::OneVector };
    int32 SegmentIndex{ 0 };
};

UCLASS()
class MPV_PRACTICAS_API UPathFollowingSteering : public UObject, public ISteering
{
    GENERATED_BODY()

public:

    // Pointer the the AICharacter used to GetSteering.
    UPROPERTY()
    TObjectPtr<AAICharacter> AICharacter;

    UFUNCTION(BlueprintCallable, Category = "Streering")
    virtual void GetSteering(FSteeringOutput& SteeringOutput) override;

    UPROPERTY()
    bool IsLooped{ false };

    UPROPERTY()
    TArray<FVector> PathPoints;

    UPROPERTY()
    int32 CurrentSegment{ 0 }; // Each segment is the union of the previust PathPoint and the next PathPoint.

    FClosestPointResult GetClosestPoint(const FVector& Position);

    FVector AdvanceAlongPath(const FClosestPointResult& Closest, float Distance);

    void DrawPath();

private:

    UPROPERTY()
    TObjectPtr<UArriveSteering> ArriveDelegate;

    UPROPERTY()
    TObjectPtr<UObstacleAvoidanceSteering> ObstacleAvoidanceDelegate;

public:

    bool EnableObstacleAvoidance{ false };
    float AvoidanceStrength{ 0.f };
    float ObstacleAvoidanceWeight{ 0.f };
};