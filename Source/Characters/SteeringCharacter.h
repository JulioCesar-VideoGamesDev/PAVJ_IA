#pragma once

#include "CoreMinimal.h"
#include "AICharacter.h"
#include "GameFramework/PlayerController.h"

#include "util/util.h"

#include "params/params.h"
#include "paths/paths.h"
#include "obstacles/obstacles.h"

#include "SteeringCharacter.generated.h"

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

// ENUMs to control the ways the Character moves.

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
class MPV_PRACTICAS_API ASteeringCharacter : public AAICharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASteeringCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Enum to control which Steering of LinearVelocity are we using.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AICharacter|Steerings|State")
	SteeringLinearVelocity StateLinearVel{ SteeringLinearVelocity::PathFollowing };

	// Enum to control which Steering of AngularVelocity are we using.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AICharacter|Steerings|State")
	SteeringAngularVelocity StateAngularVel{ SteeringAngularVelocity::AlignToMovement };

	// Enum to control which PathFinder are we using.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AICharacter|Steerings|State")
	PathFindingVersion PathFinderVersion{ PathFindingVersion::Grid };

	UPROPERTY(EditDefaultsOnly, Category = "AICharacter|Steerings|Controllers")
	bool bEnablePathfinding{ false };

	// Enables the SetActorLocation on the leftclick callback.
	UPROPERTY(EditDefaultsOnly, Category = "AICharacter|Steerings|Controllers")
	bool bEnableLeftClickTP{ false };

	// UObjectPtr for all the Steering Behaviors

	UPROPERTY(BlueprintReadOnly, Category = "AICharacter|Steerings")
	TObjectPtr<USeekSteering> SeekSteering{ nullptr };

	UPROPERTY(BlueprintReadOnly, Category = "AICharacter|Steerings")
	TObjectPtr<UArriveSteering> ArriveSteering{ nullptr };

	UPROPERTY(BlueprintReadOnly, Category = "AICharacter|Steerings")
	TObjectPtr<UAlignSteering> AlignSteering{ nullptr };

	UPROPERTY(BlueprintReadOnly, Category = "AICharacter|Steerings")
	TObjectPtr<UAlignToMovementSteering> AlignToMovementSteering{ nullptr };

	UPROPERTY(BlueprintReadOnly, Category = "AICharacter|Steerings")
	TObjectPtr<UPathFollowingSteering> PathFollowingSteering{ nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "AICharacter|Steerings|PathFollowing")
	bool bIsPathFollowingLooped{ false };

	UPROPERTY(BlueprintReadOnly, Category = "AICharacter|Steerings")
	TObjectPtr<UObstacleAvoidanceSteering> ObstacleAvoidanceSteering{ nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "AICharacter|Steerings|PathFinder")
	bool bEnableObstacleAvoidance{ false };

	// UObjectPtr for the PathFinders

	UPROPERTY(BlueprintReadOnly, Category = "AICharacter|PathFinder")
	TObjectPtr<UPathFinder_Grid> PathFinder_Grid{ nullptr };

	UPROPERTY(BlueprintReadOnly, Category = "AICharacter|PathFinder")
	TObjectPtr<UPathFinder_NavMesh> PathFinder_NavMesh{ nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "AICharacter|Steerings|PathFinder")
	FString NavMeshPath{ "" };

	FVector StartLocation{ FVector::ZeroVector };
	FVector EndLocation{ FVector::ZeroVector };
	bool bHaveEndLocation{ false };

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "AICharacter|AIFunctions")
	virtual void OnClickedLeft(const FVector& mousePosition);
	UFUNCTION(BlueprintCallable, Category = "AICharacter|AIFunctions")
	virtual void OnClickedRight(const FVector& mousePosition);

	UFUNCTION(BlueprintCallable, Category = "AICharacter|AIFunctions")
	virtual void OnPressedSpace();

	virtual void DrawDebug() override;

};
