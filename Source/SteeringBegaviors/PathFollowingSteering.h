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
    TObjectPtr<AAICharacter> AICharacter{ nullptr };

    UPROPERTY()
    bool EnableObstacleAvoidance{ false };
    
    UPROPERTY()
    float ObstacleAvoidanceStrength{ 0.f };
    
    UPROPERTY()
    float ObstacleAvoidanceWeight{ 0.f };

public:
    UFUNCTION(BlueprintCallable, Category = "Streering")
    virtual void GetSteering(FSteeringOutput& SteeringOutput) override;

    UPROPERTY()
    bool IsLooped{ false };

    UFUNCTION(BlueprintCallable, Category = "Steering")
    void ResetPathFollowingWithPath(const TArray<FVector>& NewPath, bool bResetPosition = false);

    UFUNCTION(BlueprintCallable, Category = "Steering")
    TArray<FVector> GetPathPoints() const { return PathPoints; };

    UFUNCTION(BlueprintCallable, Category = "Steering")
    bool HasFinishedPath() const;

    UFUNCTION(BlueprintCallable, Category = "Steering")
    void StopPathFollowing();

    UFUNCTION(BlueprintCallable, Category = "Steering")
    void ContinuePathFollowing();

    UFUNCTION(BlueprintCallable, Category = "Steering")
    void TogglePathFollowing();

    FClosestPointResult GetClosestPoint(const FVector& Position);

    FVector AdvanceAlongPath(const FClosestPointResult& Closest, float Distance);

    void DrawPath();

private:

    UPROPERTY()
    TObjectPtr<UArriveSteering> ArriveDelegate;

    UPROPERTY()
    TObjectPtr<UObstacleAvoidanceSteering> ObstacleAvoidanceDelegate;

    bool bHasStopedPathFollowing{ false };

    UPROPERTY()
    TArray<FVector> PathPoints;

    UPROPERTY()
    int32 CurrentSegment{ 0 }; // Each segment is the union of the previust PathPoint and the next PathPoint.
};