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

    UPROPERTY()
    AAICharacter* Character;

    virtual FSteeringOutput GetSteering() override;

    FObstacles ObstaclesStruct;
};