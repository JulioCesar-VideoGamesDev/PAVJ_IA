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

    // Pointer the the AICharacter used to GetSteering.
    UPROPERTY()
    TObjectPtr<AAICharacter> Character;

    virtual FSteeringOutput GetSteering() override;


    FVector TargetPosition{ FVector::ZeroVector };

    FVector MaxVelocityPosible = FVector::ZeroVector;
    FVector LastAcceleration = FVector::ZeroVector;

    UFUNCTION()
    void DrawDebug(FSteeringOutput Steering);
};