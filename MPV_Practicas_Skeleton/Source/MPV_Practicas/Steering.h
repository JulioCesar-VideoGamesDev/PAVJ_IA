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

/**
 * Interfaz base para cualquier comportamiento Steering.
 */
class MPV_PRACTICAS_API ISteering
{
    GENERATED_BODY()

public:

    /**
     * Calcula la aceleración lineal y angular producida por este steering.
     */
    virtual FSteeringOutput GetSteering() = 0;
};