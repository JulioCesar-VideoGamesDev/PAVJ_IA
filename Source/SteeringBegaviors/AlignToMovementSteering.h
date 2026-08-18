#pragma once

#include "CoreMinimal.h"
#include "Steering.h"
#include "AlignToMovementSteering.generated.h"

class AAICharacter;
class UAlignSteering;

UCLASS()
class MPV_PRACTICAS_API UAlignToMovementSteering : public UObject, public ISteering
{
	GENERATED_BODY()
	
public:

    // Pointer the the AICharacter used to GetSteering.
    UPROPERTY()
    TObjectPtr<AAICharacter> Character;

    virtual FSteeringOutput GetSteering() override;

private:
    UPROPERTY()
    UAlignSteering* AlignDelegate;
};
