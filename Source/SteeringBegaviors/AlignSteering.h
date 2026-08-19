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
    TObjectPtr<AAICharacter> AICharacter;

    float TargetRotation{ 0.f };

    UFUNCTION(BlueprintCallable, Category = "Streering")
    virtual void GetSteering(FSteeringOutput& SteeringOutput) override;
};
