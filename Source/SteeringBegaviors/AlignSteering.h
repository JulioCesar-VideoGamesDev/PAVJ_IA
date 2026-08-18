#pragma once

#include "CoreMinimal.h"
#include "Steering.h"
#include "AlignSteering.generated.h"

class AAICharacter;

UCLASS()
class MPV_PRACTICAS_API UAlignSteering : public UObject, public ISteering
{
	GENERATED_BODY()
	
public:

    // Pointer the the AICharacter used to GetSteering.
    UPROPERTY()
    TObjectPtr<AAICharacter> Character;

    float TargetRotation{ 0.f };

    virtual FSteeringOutput GetSteering() override;
};
