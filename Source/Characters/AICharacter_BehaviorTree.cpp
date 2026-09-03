#include "Characters/AICharacter_BehaviorTree.h"

#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

#include "../BehaviorTrees/BehaviorTree_AICharacter.h"
#include "SteeringBegaviors/PathFollowingSteering.h"
#include "PathFinding/PathFinder_Grid.h"

AAICharacter_BehaviorTree::AAICharacter_BehaviorTree()
{
    PrimaryActorTick.bCanEverTick = true;

    BehaviorTree = CreateDefaultSubobject<UBehaviorTree_AICharacter>(TEXT("BehaviorTree"));
    PathFinder_Grid = CreateDefaultSubobject<UPathFinder_Grid>(TEXT("PathFinder"));
    PathFollowingSteering = CreateDefaultSubobject<UPathFollowingSteering>(TEXT("PathFollowingBehaviour"));
}

void AAICharacter_BehaviorTree::BeginPlay()
{
    Super::BeginPlay();

    TargetActor = UGameplayStatics::GetActorOfClass(GetWorld(), TargetActorClass);

    if (!::IsValid(TargetActor))
    {
        UE_LOG(LogTemp, Error, TEXT("TargetActor NOT FOUND"));
    }

    SetupSteeringBehaviours();
    SetupBehaviorTree();
}

void AAICharacter_BehaviorTree::SetupSteeringBehaviours()
{
    if (::IsValid(PathFollowingSteering))
    {
        PathFollowingSteering->AICharacter = this;
        PathFollowingSteering->EnableObstacleAvoidance = false;
        PathFollowingSteering->ObstacleAvoidanceStrength = m_params.obstacle_avoidance_strength;
        PathFollowingSteering->ObstacleAvoidanceWeight = m_params.obstacle_avoidance_weight;
    }
}

void AAICharacter_BehaviorTree::SetupBehaviorTree()
{
    if (!::IsValid(BehaviorTree))
    {
        UE_LOG(LogTemp, Error, TEXT("BehaviorTree UObject not valid when SetupBehaviorTree"));

        return;
    }

    BuildBehaviorTree();

    UE_LOG(LogTemp, Log, TEXT("Behavior Tree initialized successfully"));
}

void AAICharacter_BehaviorTree::BuildBehaviorTree()
{
    if (!::IsValid(BehaviorTree))
    {
        UE_LOG(LogTemp, Error, TEXT("BehaviorTree UObject not valid when BuildBehaviorTree"));
        return;
    }

    UBTSelector_AIC* RootSelector = NewObject<UBTSelector_AIC>();

    // --- CHASE ---
    UBTSequence_AIC* ChaseSequence = NewObject<UBTSequence_AIC>();
    ChaseSequence->NodeName = TEXT("ChaseSequence");

        UBTDecorator_IsInChaseRange* ChaseRangeDecorator = NewObject<UBTDecorator_IsInChaseRange>();
        ChaseRangeDecorator->NodeName = TEXT("IsInChaseRange?");

            // Find path to target actor.
            UBTTask_Chase* ChaseTarget = NewObject<UBTTask_Chase>();
            ChaseTarget->NodeName = TEXT("ChaseTarget");
            
        ChaseRangeDecorator->Child = ChaseTarget;

    ChaseSequence->Children.Add(ChaseRangeDecorator);

    // --- WANDER SEQUENCE ---
    UBTSequence_AIC* WanderSequence = NewObject<UBTSequence_AIC>();
    WanderSequence->NodeName = TEXT("WanderSequence");

        // First: Wait X secods as an Idle.
        UBTDecorator_WaitTime* WaitDecorator = NewObject<UBTDecorator_WaitTime>();
        WaitDecorator->WaitTime = WaitTime;
        WaitDecorator->NodeName = TEXT("Wait 3s?");

        // Second: Generate a new path.
        UBTTask_FindPathToWander* FindPathTask = NewObject<UBTTask_FindPathToWander>();
        FindPathTask->NodeName = TEXT("FindPath");
        FindPathTask->MaxAttempts = 100;

        // Third: Follow the path.
        UBTTask_FollowPath* FollowPathTask = NewObject<UBTTask_FollowPath>();
        FollowPathTask->NodeName = TEXT("FollowPath");

    // Add all the sequence.
    WanderSequence->Children.Add(WaitDecorator);
    WanderSequence->Children.Add(FindPathTask);
    WanderSequence->Children.Add(FollowPathTask);

    // --- ROOT SELECTOR ---
    RootSelector->Children.Add(ChaseSequence);
    RootSelector->Children.Add(WanderSequence);

    BehaviorTree->RootNode = RootSelector;

    UE_LOG(LogTemp, Log, TEXT("Behavior Tree built with %d root children"), RootSelector->Children.Num());
}

void AAICharacter_BehaviorTree::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Tick the behavior tree
    if (::IsValid(BehaviorTree) && ::IsValid(BehaviorTree->RootNode))
    {
        BehaviorTree->Tick(this, DeltaTime);
    }

    FSteeringOutput Steering = FSteeringOutput();

    if (::IsValid(PathFollowingSteering) && PathFollowingSteering->GetPathPoints().Num() >= 2)
    {
        PathFollowingSteering->GetSteering(Steering);

        DrawDebugSphere(
            GetWorld(),
            PathFollowingSteering->GetPathPoints().Last(),
            20.f,
            16,
            FColor::Yellow,
            false,
            0.f
        );
    }

    ApplySteering(Steering, DeltaTime);
    DrawDebug();
}

// HELPER FUNCTIONS

void AAICharacter_BehaviorTree::GenerateWanderTarget()
{
    if (!::IsValid(PathFinder_Grid))
    {
        UE_LOG(LogTemp, Warning, TEXT("PathFinder_Grid is null, cannot generate wander target"));
        return;
    }

    FVector GridOrigin = PathFinder_Grid->GetGridOrigin();
    FVector GridSize = PathFinder_Grid->GetGridSize();

    float MinX = GridOrigin.X;
    float MaxX = GridOrigin.X + GridSize.X;
    float MinZ = GridOrigin.Z;
    float MaxZ = GridOrigin.Z + GridSize.Z;

    float RandomX = FMath::RandRange(MinX, MaxX);
    float RandomZ = FMath::RandRange(MinZ, MaxZ);

    float ConstantY = GridOrigin.Y;

    WanderTarget = FVector(RandomX, ConstantY, RandomZ);

    UE_LOG(LogTemp, Log, TEXT("Generated Wander Target: %s"), *WanderTarget.ToString());
}

float AAICharacter_BehaviorTree::GetDistanceToTargetActor()
{
    return FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
}

bool AAICharacter_BehaviorTree::IsTargetInChaseRange()
{
    return GetDistanceToTargetActor() <= ChaseRange;
}

void AAICharacter_BehaviorTree::ApplySteering(const FSteeringOutput& Steering, float DeltaTime)
{
    FVector Steering2D = FVector(Steering.Linear.X, 0.0f, Steering.Linear.Z);

    velocity += Steering2D;

    if (velocity.Length() > m_params.max_speed)
    {
        velocity = velocity.GetSafeNormal() * m_params.max_speed;
    }

    angularVelocity += Steering.Angular;

    if (FMath::Abs(angularVelocity) > m_params.max_angular_speed)
    {
        angularVelocity = FMath::Clamp(angularVelocity, -m_params.max_angular_speed, m_params.max_angular_speed);
    }

    FVector NewLocation = GetActorLocation();
    NewLocation.X += velocity.X * DeltaTime;
    NewLocation.Z += velocity.Z * DeltaTime;
    SetActorLocation(NewLocation);

    SetActorAngle(GetActorAngle() + angularVelocity * DeltaTime);

    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw = GetActorAngle();
    SetActorRotation(NewRotation);
}

void AAICharacter_BehaviorTree::DrawDebug()
{
    DrawDebugLine(
        GetWorld(),
        GetActorLocation(),
        GetActorLocation() + velocity * 0.1f,
        FColor::Green,
        false,
        0.f,
        0,
        2.0f
    );
}
