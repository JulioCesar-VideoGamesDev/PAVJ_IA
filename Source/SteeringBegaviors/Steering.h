#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SteeringOutput.h"
#include "Steering.generated.h"

UINTERFACE(MinimalAPI)
class USteering : public UInterface
{
    GENERATED_BODY()
};

// Interface for any Steering behaviour.
class MPV_PRACTICAS_API ISteering
{
    GENERATED_BODY()

public:

    // Calculates the linear acceleration and the angular acceleration.
    UFUNCTION()
    virtual FSteeringOutput GetSteering() = 0;
};