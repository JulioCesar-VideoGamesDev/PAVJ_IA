#pragma once

#include "CoreMinimal.h"
#include "Steering.h"
#include "SeekSteering.generated.h"

class AAICharacter;

UCLASS()
class MPV_PRACTICAS_API USeekSteering : public UObject, public ISteering
{
    GENERATED_BODY()

public:

    UPROPERTY()
    AAICharacter* Character;

    FVector TargetPosition;

    virtual FSteeringOutput GetSteering() override;

    FVector LastDesiredVelocity = FVector::ZeroVector;
    FVector LastAcceleration = FVector::ZeroVector;

    void DrawDebug();
};