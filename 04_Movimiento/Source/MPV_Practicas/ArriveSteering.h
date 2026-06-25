// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SeekSteering.h"
#include "ArriveSteering.generated.h"

/**
 * 
 */
UCLASS()
class MPV_PRACTICAS_API UArriveSteering : public USeekSteering
{
	GENERATED_BODY()
	
public:

	virtual FSteeringOutput GetSteering() override;
};
