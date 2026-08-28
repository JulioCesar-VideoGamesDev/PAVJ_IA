// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"

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
#include "PathFinding/PathFinder.h"

// Sets default values
AAICharacter::AAICharacter()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAICharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//----------------------------------------------------

	ReadParams("XMLs/params.xml", m_params);

	// Set initial params.
	speed = m_params.initial_speed;
	velocity = speed * direction;

	//----------------------------------------------------

	ReadPaths("XMLs/paths.xml", m_paths);
	
	// Set PathPoints.
	PathPoints = m_paths.GetPathPoints();

	//----------------------------------------------------

	ReadObstacles("XMLs/obstacles.xml", ObstaclesArray);

	//----------------------------------------------------
	
	if (bEnablePathfinding_Grid)
	{
		PathFinder = NewObject<UPathFinder>(this);

		if (::IsValid(PathFinder))
		{
			PathFinder->World = GetWorld();
			PathFinder->CellSize = 100.f;
			PathFinder->GridOrigin = FVector(-500, 0, -500);  // Origin in XZ  axis.

			if (!PathFinder->LoadGridFromFile("TXTs/grid_map.txt", "TXTs/grid_cost_config.txt"))
			{
				// If it fails we create the default grid.
				//PathFinder->SetupDefaultGrid(10, 10, 100.0f);
				UE_LOG(LogTemp, Warning, TEXT("Using default grid."));
			}
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
			PathFollowingSteering->AvoidanceStrength = m_params.obstacle_avoidance_strength;
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
void AAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	current_angle = GetActorAngle();

	// SET THE GRID

	if (PathFinder)
	{
		PathFinder->DrawGrid(true);
		PathFinder->DrawPath(PathFinder->GetLastPath());
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
void AAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAICharacter::OnClickedLeft(const FVector& mousePosition)
{
	if (bEnableLeftClickTP) SetActorLocation(mousePosition);

	if (!::IsValid(PathFinder))
	{
		return;
	}

	PathFinder->CurrentStart = PathFinder->GetCellAtLocation(mousePosition);

	// If we already have an end, we calculate the path.
	if (PathFinder->CurrentEnd)
	{
		TArray<FVector> Path = PathFinder->FindPath(PathFinder->CurrentStart->WorldLocation, PathFinder->CurrentEnd->WorldLocation);

		if (::IsValid(PathFollowingSteering))
		{
			PathFollowingSteering->ResetPathFollowingWithPath(Path);
		}
	}
}

void AAICharacter::OnClickedRight(const FVector& mousePosition)
{
	m_params.targetPosition = mousePosition;
	if (::IsValid(SeekSteering))SeekSteering->TargetPosition = mousePosition;
	if (::IsValid(ArriveSteering))ArriveSteering->TargetPosition = mousePosition;

	if (!::IsValid(PathFinder)) return;

	PathFinder->CurrentEnd = PathFinder->GetCellAtLocation(mousePosition);

	PathFinder->CurrentStart = PathFinder->GetCellAtLocation(GetActorLocation());

	if (PathFinder->CurrentStart)
	{
		TArray<FVector> Path = PathFinder->FindPath(PathFinder->CurrentStart->WorldLocation, PathFinder->CurrentEnd->WorldLocation);

		if (::IsValid(PathFollowingSteering))
		{
			PathFollowingSteering->ResetPathFollowingWithPath(Path);
		}
	}
}

void AAICharacter::OnPressedSpace()
{
	if (::IsValid(PathFollowingSteering))
	{
		PathFollowingSteering->TogglePathFollowing();
		return;
	}
}

void AAICharacter::DrawDebug()
{
	/*TArray<FVector> Points =
	{
		FVector(0.f, 0.f, 0.f),
		FVector(100.f, 0.f, 0.f),
		FVector(100.f, 0.f, 100.f),
		FVector(100.f, 0.f, 100.f),
		FVector(0.f, 0.f, 100.f)
	};*/

	//SetPath(this, TEXT("follow_path"), TEXT("path"), PathPoints, 5.0f, PathMaterial);

	//FVector dir(cos(FMath::DegreesToRadians(m_params.targetRotation)), 0.0f, sin(FMath::DegreesToRadians(m_params.targetRotation)));
	//SetArrow(this, TEXT("targetRotation"), dir, 80.0f);

	/*TArray<TArray<FVector>> Polygons = {
		{ FVector(0.f, 0.f, 0.f), FVector(100.f, 0.f, 0.f), FVector(100.f, 0.f, 100.0f), FVector(0.f, 0.f, 100.0f) },
		{ FVector(100.f, 0.f, 0.f), FVector(200.f, 0.f, 0.f), FVector(200.f, 0.f, 100.0f) }
	};
	SetPolygons(this, TEXT("navmesh"), TEXT("mesh"), Polygons, NavmeshMaterial);*/

	DrawDebugSphere(
		GetWorld(),
		m_params.targetPosition,
		5.f,
		16,              // Segments
		FColor::Green,
		false,           // Persistent
		0.f              // Duration this frame
	);

	//DrawDebugSphere(
	//	GetWorld(),
	//	ClosestPoint,
	//	40.f,
	//	16,              // Segments
	//	FColor::Green,
	//	false,           // Persistent
	//	0.f              // Duration this frame
	//);

	//DrawDebugSphere(
	//	GetWorld(),
	//	SeekPoint,
	//	40.f,
	//	16,              // Segments
	//	FColor::Yellow,
	//	false,           // Persistent
	//	0.f              // Duration this frame
	//);
}
