#include "Characters/AICharacter_StateMachine.h"

#include "Kismet/GameplayStatics.h"

#include "StateMachine/States/BaseState.h"
#include "StateMachine/StateMachine.h"
#include "../PathFinding/PathFinder_Grid.h"
#include "../SteeringBegaviors/PathFollowingSteering.h"

AAICharacter_StateMachine::AAICharacter_StateMachine()
{
    PrimaryActorTick.bCanEverTick = true;

    StateMachine = CreateDefaultSubobject<UStateMachine>(TEXT("StateMachine"));

    PathFinder_Grid = CreateDefaultSubobject<UPathFinder_Grid>(TEXT("PathFinder"));
    PathFollowingSteering = CreateDefaultSubobject<UPathFollowingSteering>(TEXT("PathFollowingBehaviour"));

    //IdleDuration = 3.0f;
    //ChaseRange = 20.0f;
}

void AAICharacter_StateMachine::BeginPlay()
{
    Super::BeginPlay();

    TargetActor = UGameplayStatics::GetActorOfClass(GetWorld(), TargetActorClass);

    if (!::IsValid(TargetActor)) UE_LOG(LogTemp, Error, TEXT("TargetActor NOT FOUND"));

    SetupSteeringBehaviours();
    SetupStateMachine();
}

void AAICharacter_StateMachine::SetupSteeringBehaviours()
{
    if (::IsValid(PathFollowingSteering))
    {
        PathFollowingSteering->AICharacter = this;
        PathFollowingSteering->EnableObstacleAvoidance = false;
        PathFollowingSteering->ObstacleAvoidanceStrength = m_params.obstacle_avoidance_strength;
        PathFollowingSteering->ObstacleAvoidanceWeight = m_params.obstacle_avoidance_weight;
    }
}

void AAICharacter_StateMachine::SetupStateMachine()
{
    if (!StateMachine)
        return;

    if (StateMachine->LoadFromXML(TEXT("XMLs/StateMachines/StateMachineConfiguration.xml")))
    {
        RegisterStateCallbacks();

        // Set up state machine delegates to call our functions
        StateMachine->Start("Idle");
        UE_LOG(LogTemp, Log, TEXT("AI State Machine initialized successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load AI State Machine configuration"));
    }
}

void AAICharacter_StateMachine::RegisterStateCallbacks()
{
    // Registras tus funciones con el StateMachine
    StateMachine->RegisterStateCallbacks(
        "Idle",
        UStateMachine::FOnStateEnter::CreateUObject(this, &AAICharacter_StateMachine::OnEnterIdle),
        UStateMachine::FOnStateExit::CreateUObject(this, &AAICharacter_StateMachine::OnExitIdle),
        UStateMachine::FOnStateUpdate::CreateUObject(this, &AAICharacter_StateMachine::OnUpdateIdle)
    );

    StateMachine->RegisterStateCallbacks(
        "Wander",
        UStateMachine::FOnStateEnter::CreateUObject(this, &AAICharacter_StateMachine::OnEnterWander),
        UStateMachine::FOnStateExit::CreateUObject(this, &AAICharacter_StateMachine::OnExitWander),
        UStateMachine::FOnStateUpdate::CreateUObject(this, &AAICharacter_StateMachine::OnUpdateWander)
    );

    StateMachine->RegisterStateCallbacks(
        "Chase",
        UStateMachine::FOnStateEnter::CreateUObject(this, &AAICharacter_StateMachine::OnEnterChase),
        UStateMachine::FOnStateExit::CreateUObject(this, &AAICharacter_StateMachine::OnExitChase),
        UStateMachine::FOnStateUpdate::CreateUObject(this, &AAICharacter_StateMachine::OnUpdateChase)
    );
}

void AAICharacter_StateMachine::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update variables for conditions
    if (!::IsValid(StateMachine)) return;

    StateMachine->SetFloatVariable("IdleTimer", IdleTimer);
    StateMachine->SetFloatVariable("DistanceToTargetActor", GetDistanceToTargetActor());
    StateMachine->SetBoolVariable("IsTargetActorInChaseRange", IsTargetActorInChaseRange());
    StateMachine->SetBoolVariable("HasReachedTarget", ReachedTargetLocation);

    // Update the state machine to change the current state in case we have to.
    StateMachine->Update(DeltaTime);

    TObjectPtr<UBaseState> CurrentState = StateMachine->CurrentState;
    FString NameCurrentState = CurrentState->Name;

    FSteeringOutput Steering = FSteeringOutput();
    if (NameCurrentState != "Idle" && ::IsValid(PathFollowingSteering) && PathFollowingSteering->GetPathPoints().Num() >= 2)
    {
        PathFollowingSteering->GetSteering(Steering);

        DrawDebugSphere(
            GetWorld(),
            PathFollowingSteering->GetPathPoints().Last(),
            20.f,
            16,              // Segments
            FColor::Yellow,
            false,           // Persistent
            0.f              // Duration this frame
        );
    }

    ApplySteering(Steering, DeltaTime);
}

void AAICharacter_StateMachine::OnEnterIdle()
{
    IdleTimer = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("Entering Idle state"));
}

void AAICharacter_StateMachine::OnUpdateIdle(const float DeltaTime)
{
    IdleTimer += DeltaTime;
}

void AAICharacter_StateMachine::OnExitIdle()
{
    IdleTimer = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("Exiting Idle state"));
}

void AAICharacter_StateMachine::OnEnterWander()
{
    GenerateWanderTarget();

    // Find path to wander target
    TArray<FVector> Path = PathFinder_Grid->FindPath(GetActorLocation(), WanderTarget);
    PathFollowingSteering->ResetPathFollowingWithPath(Path);

    UE_LOG(LogTemp, Log, TEXT("Entering Wander state - Target: %s"), *WanderTarget.ToString());
}

void AAICharacter_StateMachine::OnUpdateWander(const float DeltaTime)
{
    if (PathFollowingSteering->HasFinishedPath())
        ReachedTargetLocation = true;
}

void AAICharacter_StateMachine::OnExitWander()
{
    ReachedTargetLocation = false;
    UE_LOG(LogTemp, Log, TEXT("Exiting Wander state"));
}

void AAICharacter_StateMachine::OnEnterChase()
{
    UE_LOG(LogTemp, Log, TEXT("Entering Chase state"));
}

void AAICharacter_StateMachine::OnUpdateChase(const float DeltaTime)
{
    if (TargetActor)
    {
        FVector PlayerLocation = TargetActor->GetActorLocation();

        // Find path to player
        TArray<FVector> Path = PathFinder_Grid->FindPath(GetActorLocation(), PlayerLocation);
        PathFollowingSteering->ResetPathFollowingWithPath(Path);
    }
}

void AAICharacter_StateMachine::OnExitChase()
{
    UE_LOG(LogTemp, Log, TEXT("Exiting Chase state"));
}

void AAICharacter_StateMachine::GenerateWanderTarget()
{
    if (!PathFinder_Grid)
    {
        UE_LOG(LogTemp, Warning, TEXT("PathFinder_Grid is null, cannot generate wander target"));
        return;
    }

    // Get the grid boundaries
    FVector GridOrigin = PathFinder_Grid->GetGridOrigin();
    FVector GridSize = FVector(PathFinder_Grid->GetGridSize().X, GridOrigin.Y, PathFinder_Grid->GetGridSize().Z);
    float CellSize = PathFinder_Grid->GetCellSize();

    // Calculate the minimum and maximum coordinates of the grid
    float MinX = GridOrigin.X;
    float MaxX = GridOrigin.X + GridSize.X;
    float MinZ = GridOrigin.Z;
    float MaxZ = GridOrigin.Z + GridSize.Z;

    // Generate a random point within the grid area
    float RandomX = FMath::RandRange(MinX, MaxX);
    float RandomZ = FMath::RandRange(MinZ, MaxZ);

    float CurrentY = GetActorLocation().Y;

    WanderTarget = FVector(RandomX, CurrentY, RandomZ);

    UE_LOG(LogTemp, Log, TEXT("Generated Wander Target: %s (Grid bounds: X[%.0f-%.0f] Z[%.0f-%.0f])"),
        *WanderTarget.ToString(), MinX, MaxX, MinZ, MaxZ);
}

float AAICharacter_StateMachine::GetDistanceToTargetActor()
{
    if (!::IsValid(TargetActor))
        return 9999.0f;

    return FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
}

bool AAICharacter_StateMachine::IsTargetActorInChaseRange()
{
    return GetDistanceToTargetActor() <= ChaseRange;
}

void AAICharacter_StateMachine::ApplySteering(const FSteeringOutput& Steering, const float DeltaTime)
{
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

    DrawDebug();
}
