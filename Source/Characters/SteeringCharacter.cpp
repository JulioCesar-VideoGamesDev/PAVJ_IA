#include "Characters/SteeringCharacter.h"

#include "debug/debugdraw.h"

// Linear velocity
#include "SteeringBegaviors/SeekSteering.h"
#include "SteeringBegaviors/ArriveSteering.h"

// Angular velocity
#include "SteeringBegaviors/AlignSteering.h"
#include "SteeringBegaviors/AlignToMovementSteering.h"

// Advanced Steerings
#include "SteeringBegaviors/PathFollowingSteering.h"
#include "SteeringBegaviors/ObstacleAvoidanceSteering.h"

// PathFinding
#include "PathFinding/PathFinder_Grid.h"
#include "PathFinding/PathFinder_NavMesh.h"

// Sets default values
ASteeringCharacter::ASteeringCharacter()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASteeringCharacter::BeginPlay()
{
	Super::BeginPlay();

	//----------------------------------------------------

	ReadPaths("XMLs/paths/paths.xml", m_paths);

	// Set PathPoints.
	PathPoints = m_paths.GetPathPoints();

	//----------------------------------------------------

	if (bEnablePathfinding)
	{
		switch (PathFinderVersion)
		{
		case PathFindingVersion::Grid:
			PathFinder_Grid = NewObject<UPathFinder_Grid>(this);

			break;
		case PathFindingVersion::NavMesh:
			PathFinder_NavMesh = NewObject<UPathFinder_NavMesh>(this);

			if (::IsValid(PathFinder_NavMesh))
			{
				PathFinder_NavMesh->World = GetWorld();
				PathFinder_NavMesh->HeightOffset = 1.f;  // Altura del suelo

				if (!PathFinder_NavMesh->LoadNavMeshFromFile(NavMeshPath))
				{
					// If it fails we create the default grid.
					//PathFinder_NavMesh->SetupDefaultGrid(10, 10, 100.0f);
					UE_LOG(LogTemp, Warning, TEXT("Using default grid."));
				}
			}

			break;
		default:
			break;
		}
	}

	//----------------------------------------------------

	// INITIALIZE STEERINGS

	switch (StateLinearVel)
	{
	case SteeringLinearVelocity::Seek:
		SeekSteering = NewObject<USeekSteering>(this);

		if (::IsValid(SeekSteering))
		{
			SeekSteering->AICharacter = this;
			SeekSteering->TargetPosition = m_params.targetPosition;
			SeekSteering->DoDrawDebug = true;
		}

		break;
	case SteeringLinearVelocity::Arrive:
		ArriveSteering = NewObject<UArriveSteering>(this);

		if (::IsValid(ArriveSteering))
		{
			ArriveSteering->AICharacter = this;
			ArriveSteering->TargetPosition = m_params.targetPosition;
			ArriveSteering->DoDrawDebug = true;
			ArriveSteering->BrakeMinSpeed = m_params.brake_min_speed;
		}

		break;
	case SteeringLinearVelocity::PathFollowing:
		PathFollowingSteering = NewObject<UPathFollowingSteering>(this);

		if (::IsValid(PathFollowingSteering))
		{
			PathFollowingSteering->AICharacter = this;
			PathFollowingSteering->ResetPathFollowingWithPath(m_paths.PathPoints);
			PathFollowingSteering->DrawPath();
			PathFollowingSteering->IsLooped = bIsPathFollowingLooped;

			PathFollowingSteering->EnableObstacleAvoidance = bEnableObstacleAvoidance;
			PathFollowingSteering->ObstacleAvoidanceWeight = m_params.obstacle_avoidance_weight;
			PathFollowingSteering->ObstacleAvoidanceStrength = m_params.obstacle_avoidance_strength;
		}

		break;
	default:
		break;
	}

	switch (StateAngularVel)
	{
	case SteeringAngularVelocity::Align:
		AlignSteering = NewObject<UAlignSteering>(this);

		if (::IsValid(AlignSteering))
		{
			AlignSteering->AICharacter = this;
			AlignSteering->TargetRotation = m_params.targetRotation;
		}

		break;
	case SteeringAngularVelocity::AlignToMovement:
		AlignToMovementSteering = NewObject<UAlignToMovementSteering>(this);

		if (::IsValid(AlignToMovementSteering))
		{
			AlignToMovementSteering->AICharacter = this;
		}

		break;
	default:
		break;
	}

	if (bEnableObstacleAvoidance && !::IsValid(PathFollowingSteering))
	{
		ObstacleAvoidanceSteering = NewObject<UObstacleAvoidanceSteering>(this);
		ObstacleAvoidanceSteering->AICharacter = this;
		ObstacleAvoidanceSteering->ObstaclesArray = ObstaclesArray;
		ObstacleAvoidanceSteering->AvoidanceStrength = m_params.obstacle_avoidance_strength;
		ObstacleAvoidanceSteering->DoDrawDebug = true;
	}
}

// Called every frame
void ASteeringCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	current_angle = GetActorAngle();

	// SET THE GRID

	if (::IsValid(PathFinder_Grid))
	{
		PathFinder_Grid->DrawGrid(true);
		PathFinder_Grid->DrawPath(PathFinder_Grid->GetLastPath());
	}

	if (::IsValid(PathFinder_NavMesh))
	{
		PathFinder_NavMesh->DrawPath(PathFinder_NavMesh->GetLastPath());
	}

	// GET THE STEERING
	FSteeringOutput Steering;

	if (::IsValid(SeekSteering))
	{
		SeekSteering->GetSteering(Steering);
	}
	else if (::IsValid(ArriveSteering))
	{
		ArriveSteering->GetSteering(Steering);
	}
	else if (::IsValid(PathFollowingSteering))
	{
		PathFollowingSteering->GetSteering(Steering);
	}

	if (::IsValid(ObstacleAvoidanceSteering))
	{
		FSteeringOutput ObstacleAvoidanceSteeringResult = ObstacleAvoidanceSteering->FindCollisionAndGetSteering(Steering);

		if (ObstacleAvoidanceSteering->GetFoundCollision().bWillCollide)
		{
			FSteeringOutput CurrentSteering = Steering;

			Steering.Linear =
				CurrentSteering.Linear +
				ObstacleAvoidanceSteeringResult.Linear * m_params.obstacle_avoidance_weight;

			Steering.Linear.Normalize();
			Steering.Linear *= m_params.max_acceleration;
		}
	}

	velocity += Steering.Linear;

	if (velocity.Length() > m_params.max_speed) // Clamp the velocity with m_params.max_velocity
	{
		velocity =
			velocity.GetSafeNormal() *
			m_params.max_speed;
	}

	// Set the new Location based on our Velocity.
	SetActorLocation(
		GetActorLocation() +
		velocity * DeltaTime);

	if (::IsValid(AlignSteering))
	{
		AlignSteering->GetSteering(Steering);
	}
	else if (::IsValid(AlignToMovementSteering))
	{
		AlignToMovementSteering->GetSteering(Steering);
	}

	angularVelocity += Steering.Angular;

	if (FMath::Abs(angularVelocity) > m_params.max_angular_speed)
	{
		if (angularVelocity >= 0) angularVelocity = m_params.max_angular_speed;
		else angularVelocity = m_params.max_angular_speed * -1;
	}

	// Set the new Rotation based on our AngularVelocity.
	SetActorAngle(
		GetActorAngle() +
		angularVelocity * DeltaTime);

	// Draw some Debugs to visualize better whats happening.
	DrawDebug();
}

// Called to bind functionality to input
void ASteeringCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASteeringCharacter::OnClickedLeft(const FVector& mousePosition)
{
	if (bEnableLeftClickTP) SetActorLocation(mousePosition);

	if (::IsValid(PathFinder_Grid))
	{
		PathFinder_Grid->CurrentStart = PathFinder_Grid->GetCellAtLocation(mousePosition);

		// If we already have an end, we calculate the path.
		if (PathFinder_Grid->CurrentEnd && PathFinder_Grid->CurrentStart)
		{
			TArray<FVector> Path = PathFinder_Grid->FindPath(PathFinder_Grid->CurrentStart->WorldLocation, PathFinder_Grid->CurrentEnd->WorldLocation);

			if (::IsValid(PathFollowingSteering))
			{
				PathFollowingSteering->ResetPathFollowingWithPath(Path);
			}
		}
	}

	if (::IsValid(PathFinder_NavMesh))
	{
		StartLocation = mousePosition;

		if (!bHaveEndLocation) return;

		TArray<FVector> Path = PathFinder_NavMesh->FindPath(StartLocation, EndLocation);

		if (Path.Num() > 0)
		{
			// Enviar al sistema de PathFollowing
			PathFollowingSteering->ResetPathFollowingWithPath(Path);
		}
	}
}

void ASteeringCharacter::OnClickedRight(const FVector& mousePosition)
{
	m_params.targetPosition = mousePosition;
	if (::IsValid(SeekSteering))SeekSteering->TargetPosition = mousePosition;
	if (::IsValid(ArriveSteering))ArriveSteering->TargetPosition = mousePosition;

	if (::IsValid(PathFinder_Grid))
	{
		PathFinder_Grid->CurrentEnd = PathFinder_Grid->GetCellAtLocation(mousePosition);

		PathFinder_Grid->CurrentStart = PathFinder_Grid->GetCellAtLocation(GetActorLocation());

		if (PathFinder_Grid->CurrentStart && PathFinder_Grid->CurrentEnd)
		{
			TArray<FVector> Path = PathFinder_Grid->FindPath(PathFinder_Grid->CurrentStart->WorldLocation, PathFinder_Grid->CurrentEnd->WorldLocation);

			if (::IsValid(PathFollowingSteering))
			{
				PathFollowingSteering->ResetPathFollowingWithPath(Path);
			}
		}
	}

	if (::IsValid(PathFinder_NavMesh))
	{
		StartLocation = GetActorLocation();
		EndLocation = mousePosition;
		bHaveEndLocation = true;

		TArray<FVector> Path = PathFinder_NavMesh->FindPath(StartLocation, EndLocation);

		if (Path.Num() > 0)
		{
			// Enviar al sistema de PathFollowing
			PathFollowingSteering->ResetPathFollowingWithPath(Path);
		}
	}
}

void ASteeringCharacter::OnPressedSpace()
{
	if (::IsValid(PathFollowingSteering))
	{
		PathFollowingSteering->TogglePathFollowing();
		return;
	}
}

void ASteeringCharacter::DrawDebug()
{
	Super::DrawDebug();

	if (::IsValid(PathFinder_NavMesh))
	{
		TArray<TArray<FVector>> Polygons = PathFinder_NavMesh->GetNavMeshPolygons();

		if (Polygons.Num() > 1)
		{
			TArray<TArray<FVector>> PolygonsTest = {
				{ FVector(0.f, 0.f, 0.f), FVector(100.f, 0.f, 0.f), FVector(100.f, 0.f, 100.0f), FVector(0.f, 0.f, 200.0f) },
				{ FVector(0.f, 0.f, 0.f), FVector(100.f, 0.f, 0.f), FVector(100.f, 0.f, 100.0f), FVector(0.f, 0.f, 200.0f) }
			};
			SetPolygons(this, TEXT("navmesh"), TEXT("mesh"), PolygonsTest, NavmeshMaterial);
		}
	}
}
