// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "params/params.h"
#include "GameFramework/PlayerController.h"
#include "util.h"


#include "AICharacter.generated.h"

class USeekSteering;
class UArriveSteering;

class UAlignSteering;

class UArrowComponent;

UCLASS()
class MPV_PRACTICAS_API AAICharacter : public APawn
{
	GENERATED_BODY()
private:

	float speed{ 0.f };

	FVector direction = FVector::ForwardVector;

	FVector velocity = FVector::ZeroVector;

	float angularVelocity { 0.f };

public:
	// Sets default values for this pawn's properties
	AAICharacter();

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AIChar)
	uint32 bDoMovement : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AIChar)
		float current_angle;
		
	UPROPERTY()
	USeekSteering* SeekSteering{ nullptr };

	UPROPERTY()
	UArriveSteering* ArriveSteering{ nullptr };

	UPROPERTY()
	UAlignSteering* AlignSteering{ nullptr };

	UPROPERTY(EditAnywhere)
		UMaterialInterface* PathMaterial{ nullptr };

	UPROPERTY(EditAnywhere)
		UMaterialInterface* NavmeshMaterial{ nullptr };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	
	Params m_params;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "AIFunctions")
	void OnClickedLeft(const FVector& mousePosition);
	UFUNCTION(BlueprintCallable, Category = "AIFunctions")
	void OnClickedRight(const FVector& mousePosition);
	
	const Params& GetParams() const { return m_params; }

	float GetActorAngle() const
	{
		FQuat currQuat = GetActorQuat();
		FVector axis;
		float axisAngle;
		currQuat.ToAxisAndAngle(axis, axisAngle);
		axisAngle = FMath::RadiansToDegrees(axisAngle);
		if (axis.Y > 0.0f)
			axisAngle = -axisAngle;

		axisAngle = convertTo360(axisAngle);
		return axisAngle;
	}
	void SetActorAngle(float angle) { FRotator newRot(angle, 0.0f, 0.0f); SetActorRotation(newRot); }

	FVector GetCurrentVelocity() const
	{
		return velocity;
	}

	float GetCurrentAngularVelocity() const
	{
		return angularVelocity;
	}

	void DrawDebug();

};
