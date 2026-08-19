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
    TObjectPtr<AAICharacter> AICharacter;

    UFUNCTION(BlueprintCallable, Category = "Streering")
    virtual void GetSteering(FSteeringOutput& SteeringOutput) override;

private:
    UPROPERTY()
    TObjectPtr<UAlignSteering> AlignDelegate;
};
