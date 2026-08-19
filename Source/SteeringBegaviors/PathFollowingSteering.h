#pragma once

#include "CoreMinimal.h"
#include "Steering.h"
#include "PathFollowingSteering.generated.h"

class AAICharacter;
class USeekSteering;

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



    TArray<FVector> PathPoints;

    FVector ClosestPointOnSegment(const FVector& Point, const FVector& A, const FVector& B);

    FClosestPointResult GetClosestPoint(const FVector& Position);

    FVector AdvanceAlongPath(const FClosestPointResult& Closest, float Distance);

    //FVector LastDesiredVelocity = FVector::ZeroVector;
    //FVector LastAcceleration = FVector::ZeroVector;

private:
    UPROPERTY()
    TObjectPtr<USeekSteering> SeekDelegate;
};