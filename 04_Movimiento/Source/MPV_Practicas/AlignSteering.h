// Fill out your copyright notice in the Description page of Project Settings.

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

    UPROPERTY()
    AAICharacter* Character;

    float TargetRotation;

    virtual FSteeringOutput GetSteering() override;
};
