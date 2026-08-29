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

// PathFinding
class UPathFinder_Grid;
class UPathFinder_NavMesh;

UENUM(BlueprintType)
enum class SteeringLinearVelocity : uint8
{
	Seek UMETA(DisplayName = "Seek"),
	Arrive UMETA(DisplayName = "Arrive"),
	PathFollowing UMETA(DisplayName = "PathFollowing")
};

UENUM(BlueprintType)
enum class SteeringAngularVelocity : uint8
{
	Align UMETA(DisplayName = "Align"),
	AlignToMovement UMETA(DisplayName = "AlignToMovement")
};

UENUM(BlueprintType)
enum class PathFindingVersion : uint8
{
	Grid UMETA(DisplayName = "Grid"),
	NavMesh UMETA(DisplayName = "NavMesh")
};

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
	
	// Enum to control which Steering of LinearVelocity are we using.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steerings|State")
	SteeringLinearVelocity StateLinearVel{ SteeringLinearVelocity::PathFollowing };

	// Enum to control which Steering of AngularVelocity are we using.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steerings|State")
	SteeringAngularVelocity StateAngularVel{ SteeringAngularVelocity::AlignToMovement };
	
	// Enum to control which PathFinder are we using.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steerings|State")
	PathFindingVersion PathFinderVersion{ PathFindingVersion::Grid };

	UPROPERTY(EditDefaultsOnly, Category = "Steerings|Controllers")
	bool bEnablePathfinding{ false };

	// Enables the SetActorLocation on the leftclick callback.
	UPROPERTY(EditDefaultsOnly, Category = "Steerings|Controllers")
	bool bEnableLeftClickTP{ false };

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
	
	UPROPERTY(EditDefaultsOnly, Category = "Steerings|PathFollowing")
	bool bIsPathFollowingLooped{ false };

	UPROPERTY(BlueprintReadOnly, Category = "Steerings")
	TObjectPtr<UObstacleAvoidanceSteering> ObstacleAvoidanceSteering;

	UPROPERTY(EditDefaultsOnly, Category = "Steerings|PathFinder")
	bool bEnableObstacleAvoidance{ false };

	UPROPERTY(BlueprintReadOnly, Category = "PathFinder")
	TObjectPtr<UPathFinder_Grid> PathFinder_Grid;
	
	UPROPERTY(EditDefaultsOnly, Category = "Steerings|PathFinder")
	FVector GridOrigin { FVector::ZeroVector };

	UPROPERTY(BlueprintReadOnly, Category = "PathFinder")
	TObjectPtr<UPathFinder_NavMesh> PathFinder_NavMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Steerings|PathFinder")
	FString NavMeshPath{ "" };

	FVector StartLocation{ FVector::ZeroVector };
	FVector EndLocation{ FVector::ZeroVector };
	bool bHaveEndLocation{ false };

protected:

	float speed{ 0.f };

	FVector direction = FVector::ForwardVector;

	FVector velocity = FVector::ZeroVector;

	float angularVelocity{ 0.f };


	TArray<FVector> PathPoints;
	TArray<FObstacleAttr> ObstaclesArray;

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

	UFUNCTION(BlueprintCallable, Category = "AIFunctions")
	void OnPressedSpace();
	
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

	TArray<FObstacleAttr> GetObstaclesArray() const
	{
		return ObstaclesArray;
	}
};
