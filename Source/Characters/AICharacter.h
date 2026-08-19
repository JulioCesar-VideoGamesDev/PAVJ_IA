#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include "util/util.h"

#include "params/params.h"
#include "paths/paths.h"
#include "obstacles/obstacles.h"

#include "AICharacter.generated.h"

// Linear velocity
class USeekSteering;
class UArriveSteering;

// Angular velocity
class UAlignSteering;
class UAlignToMovementSteering;

// Advanced Steerings
class UPathFollowingSteering;
class UObstacleAvoidanceSteering;

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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Struct that stores all the params defined in Content/XMLs/params.xml
	Params m_params;

	// Struct that stores all the points defined in Content/XMLs/paths.xml
	Paths m_paths;
	
	// Struct that stores all the obstacles defined in Content/XMLs/obstacles.xml
	FObstacles m_obstacles;;
	
	// UObjectPtr for all the Steering Behaviors

	UPROPERTY(BlueprintReadOnly, Category = "Steerings")
	TObjectPtr<USeekSteering> SeekSteering;
	
	UPROPERTY(BlueprintReadOnly, Category = "Steerings")
	TObjectPtr<UArriveSteering> ArriveSteering;
	
	UPROPERTY(BlueprintReadOnly, Category = "Steerings")
	TObjectPtr<UAlignSteering> AlignSteering;
	
	UPROPERTY(BlueprintReadOnly, Category = "Steerings")
	TObjectPtr<UAlignToMovementSteering> AlignToMovementSteering;

	UPROPERTY(BlueprintReadOnly, Category = "Steerings")
	TObjectPtr<UPathFollowingSteering> PathFollowingSteering;
	
	UPROPERTY(BlueprintReadOnly, Category = "Steerings")
	TObjectPtr<UObstacleAvoidanceSteering> ObstacleAvoidanceSteering;

protected:

	float speed{ 0.f };

	FVector direction = FVector::ForwardVector;

	FVector velocity = FVector::ZeroVector;

	float angularVelocity{ 0.f };


	TArray<FVector> PathPoints;
	TArray<ObstacleAttr> ObstaclesArray;

	FVector ClosestPoint;
	FVector SeekPoint;
	FVector PredictedPoint;

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

	void DrawDebug();

	// FUNCTIONS FOR THE COMUNICATION WITH THE STEERINGS -------------------------------------------------------------------------------------------------------------------

	FVector GetTargetPosition() const
	{
		return m_params.targetPosition;
	}
	
	float GetTargetRotation() const
	{
		return m_params.targetRotation;
	}

	FVector GetAICharacterCurrentVelocity() const
	{
		return velocity;
	}

	float GetAICharacterCurrentAngularVelocity() const
	{
		return angularVelocity;
	}

	TArray<FVector> GetPathPoints() const
	{
		return PathPoints;
	}
};
