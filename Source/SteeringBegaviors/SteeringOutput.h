#pragma once

#include "CoreMinimal.h"
#include "SteeringOutput.generated.h"

USTRUCT(BlueprintType)
struct FSteeringOutput
{
    GENERATED_BODY()

public:

    // Aceleración lineal (m/s²)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Linear = FVector::ZeroVector;

    // Aceleración angular (rad/s² o deg/s² según vuestra convención)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Angular = 0.0f;
};