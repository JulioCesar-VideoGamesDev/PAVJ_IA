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
	
	PathFinder = NewObject<UPathFinder>(this);

	if (::IsValid(PathFinder))
	{
		PathFinder->World = GetWorld();
		PathFinder->CellSize = 100.f;
		PathFinder->GridOrigin = FVector(-500, 0, -500);  // Origin in XZ  axis.

		if (!PathFinder->LoadGridFromFile("TXTs/grid_map.txt", "TXTs/grid_cost_config.txt"))
		{
			// If it fails we create the default grid.
			PathFinder->SetupDefaultGrid(10, 10, 100.0f);
			UE_LOG(LogTemp, Warning, TEXT("Using default grid."));
		}
	}
	
	//----------------------------------------------------

	// INITIALIZE STEERINGS

	PathFollowingSteering = NewObject<UPathFollowingSteering>(this);
	
	if (::IsValid(PathFollowingSteering))
	{
		PathFollowingSteering->AICharacter = this;
		PathFollowingSteering->ResetPathFollowingWithPath(m_paths.PathPoints);
		PathFollowingSteering->DrawPath();
		PathFollowingSteering->IsLooped = false;

		PathFollowingSteering->EnableObstacleAvoidance = true;
		//PathFollowingSteering->AvoidanceStrength = 1000.f;
		PathFollowingSteering->ObstacleAvoidanceWeight = 20.f;
	}

	//ObstacleAvoidanceSteering = NewObject<UObstacleAvoidanceSteering>(this);

	if (::IsValid(ObstacleAvoidanceSteering))
	{
		ObstacleAvoidanceSteering->AICharacter = this;
		ObstacleAvoidanceSteering->DoDrawDebug = true;
	}

	AlignToMovementSteering = NewObject<UAlignToMovementSteering>(this);

	if (::IsValid(AlignToMovementSteering))
	{
		AlignToMovementSteering->AICharacter = this;
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

	if (::IsValid(PathFollowingSteering))
	{
		PathFollowingSteering->GetSteering(Steering);
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

	if (AlignToMovementSteering)
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
	//SetActorLocation(mousePosition);

	if (!::IsValid(PathFinder)) return;

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

	TArray<TArray<FVector>> Polygons = {
		{ FVector(0.f, 0.f, 0.f), FVector(100.f, 0.f, 0.f), FVector(100.f, 0.f, 100.0f), FVector(0.f, 0.f, 100.0f) },
		{ FVector(100.f, 0.f, 0.f), FVector(200.f, 0.f, 0.f), FVector(200.f, 0.f, 100.0f) }
	};
	SetPolygons(this, TEXT("navmesh"), TEXT("mesh"), Polygons, NavmeshMaterial);

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
