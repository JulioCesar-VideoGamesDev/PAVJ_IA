#pragma once

#include "CoreMinimal.h"
#include "Steering.h"
#include "obstacles/obstacles.h"
#include "ObstacleAvoidanceSteering.generated.h"

class AAICharacter;

UCLASS()
class MPV_PRACTICAS_API UObstacleAvoidanceSteering : public UObject, public ISteering
{
    GENERATED_BODY()

public:

    // Pointer the the AICharacter used to GetSteering.
    UPROPERTY()
    TObjectPtr<AAICharacter> AICharacter;

    UFUNCTION(BlueprintCallable, Category = "Streering")
    virtual void GetSteering(FSteeringOutput& SteeringOutput) override;

    FObstacles ObstaclesStruct;
};