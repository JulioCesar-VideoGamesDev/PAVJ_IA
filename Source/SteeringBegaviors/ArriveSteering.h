#pragma once

#include "CoreMinimal.h"
#include "SeekSteering.h"
#include "ArriveSteering.generated.h"

UCLASS()
class MPV_PRACTICAS_API UArriveSteering : public USeekSteering
{
	GENERATED_BODY()
	
public:

	float BrakeMinSpeed{ 0.f };

	virtual void GetSteering(FSteeringOutput& SteeringOutput) override;
};
