// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "util.h"
#include "params/params.h"
#include "paths/paths.h"
#include "AICharacter.generated.h"

class USeekSteering;

UCLASS()
class MPV_PRACTICAS_API AAICharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AAICharacter();

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AIChar)
	uint32 bDoMovement : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AIChar)
		float current_angle;

	UPROPERTY(EditAnywhere)
		UMaterialInterface* PathMaterial;

	UPROPERTY(EditAnywhere)
		UMaterialInterface* NavmeshMaterial;

private:

	float speed{ 0.f };

	FVector direction = FVector::ForwardVector;

	FVector velocity = FVector::ZeroVector;

	float angularVelocity{ 0.f };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	Params m_params;

	Paths m_paths;

	USeekSteering* SeekSteering;

protected:

	TArray<FVector> PathPoints;

	FVector ClosestPoint;
	FVector SeekPoint;
	FVector PredictedPoint;

	FVector Velocity = FVector::ZeroVector;

public:	
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

	FVector GetClosestPointOnSegment(const FVector& P, const FVector& A, const FVector& B);
	int GetClosestPathPoint(const FVector& Position, FVector& OutPoint);
	FVector GetLookAheadPoint(int Segment, const FVector& StartPoint, float DistanceAhead);

	void PathFollowing();
};
