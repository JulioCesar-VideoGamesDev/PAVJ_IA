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
#include "PathFinding/PathFinder_Grid.h"
#include "PathFinding/PathFinder_NavMesh.h"

// StateMachine
#include "StateMachine/StateMachine.h"

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

	ReadParams(TCHAR_TO_UTF8(*m_params_filePath), m_params);

	// Set initial params.
	speed = m_params.initial_speed;
	velocity = speed * direction;

	//----------------------------------------------------

	ReadObstacles(TCHAR_TO_UTF8(*ObstaclesArray_filePath), ObstaclesArray);
}

// Called every frame
void AAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	current_angle = GetActorAngle();

	// Draw some Debugs to visualize better whats happening.
	DrawDebug();
}

void AAICharacter::DrawDebug()
{
	DrawDebugSphere(
		GetWorld(),
		m_params.targetPosition,
		5.f,
		16,              // Segments
		FColor::Green,
		false,           // Persistent
		0.f              // Duration this frame
	);
}
