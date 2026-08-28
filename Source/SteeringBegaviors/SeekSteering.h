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
    TObjectPtr<AAICharacter> AICharacter;

    UFUNCTION(BlueprintCallable, Category = "Streering")
    virtual void GetSteering(FSteeringOutput& SteeringOutput) override;


    FVector TargetPosition{ FVector::ZeroVector };

    /*FVector MaxVelocityPosible = FVector::ZeroVector;
    FVector LastAcceleration = FVector::ZeroVector;*/

    bool DoDrawDebug{ false };

    UFUNCTION()
    void DrawDebug(FSteeringOutput Steering);
};