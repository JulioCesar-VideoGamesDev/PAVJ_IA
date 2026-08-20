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

	ReadObstacles("XMLs/obstacles.xml", m_obstacles);

	// Set ObstaclesArray.
	ObstaclesArray = m_obstacles.GetObstaclesArray();

	//----------------------------------------------------

	// INITIALIZE STEERINGS

	PathFollowingSteering = NewObject<UPathFollowingSteering>(this);
	
	if (PathFollowingSteering)
	{
		PathFollowingSteering->AICharacter = this;
		PathFollowingSteering->PathPoints = m_paths.PathPoints;
		PathFollowingSteering->IsLooped = true;
	}

	AlignToMovementSteering = NewObject<UAlignToMovementSteering>(this);

	if (AlignToMovementSteering)
	{
		AlignToMovementSteering->AICharacter = this;
	}
}

// Called every frame
void AAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	current_angle = GetActorAngle();

	// GET THE STEERING
	FSteeringOutput Steering;

	if (PathFollowingSteering)
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
	SetActorLocation(mousePosition);
}

void AAICharacter::OnClickedRight(const FVector& mousePosition)
{
	m_params.targetPosition = mousePosition;

	FVector dir = (mousePosition - GetActorLocation()).GetSafeNormal();
	/*float angle = FMath::RadiansToDegrees(atan2(dir.Z, dir.X));
	m_params.targetRotation = angle;*/
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

	SetPath(this, TEXT("follow_path"), TEXT("path"), PathPoints, 5.0f, PathMaterial);

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
		20.f,
		16,              // Segments
		FColor::Red,
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
