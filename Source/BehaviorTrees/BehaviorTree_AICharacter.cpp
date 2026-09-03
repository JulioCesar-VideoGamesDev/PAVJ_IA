#include "BehaviorTrees/BehaviorTree_AICharacter.h"
#include "Characters/AICharacter_BehaviorTree.h"
#include "SteeringBegaviors/PathFollowingSteering.h"
#include "PathFinding/PathFinder_Grid.h"

// SELECTOR IMPLEMENTATION

UBTSelector_AIC::UBTSelector_AIC()
{
    CurrentChildIndex = 0;
    LastExecutedChildIndex = -1;
    NodeName = TEXT("Selector");
}

EBTNodeStatus_AIC UBTSelector_AIC::Execute(AAICharacter_BehaviorTree* Owner)
{
    if (!Owner || Children.Num() == 0)
        return EBTNodeStatus_AIC::Failure;

    for (int32 i = 0; i < Children.Num(); i++)
    {
        UBTNode_AIC* Child = Children[i];
        if (!Child)
            continue;

        EBTNodeStatus_AIC Status = Child->Execute(Owner);

        if (Status == EBTNodeStatus_AIC::Running)
        {
            if (LastExecutedChildIndex != -1 && LastExecutedChildIndex != i)
            {
                Children[LastExecutedChildIndex]->Reset();
                LastExecutedChildIndex = i;
            }

            CurrentChildIndex = i;
            LastExecutedChildIndex = CurrentChildIndex;

            return EBTNodeStatus_AIC::Running;
        }

        if (Status == EBTNodeStatus_AIC::Success)
        {
            Reset();
            return EBTNodeStatus_AIC::Success;
        }

    }

    // All the childs failed.
    Reset();
    return EBTNodeStatus_AIC::Failure;
}

void UBTSelector_AIC::Reset()
{
    CurrentChildIndex = 0;
    for (UBTNode_AIC* Child : Children)
    {
        if (Child)
            Child->Reset();
    }
}

// SEQUENCE IMPLEMENTATION

UBTSequence_AIC::UBTSequence_AIC()
{
    CurrentChildIndex = 0;
    NodeName = TEXT("Sequence");
}

EBTNodeStatus_AIC UBTSequence_AIC::Execute(AAICharacter_BehaviorTree* Owner)
{
    if (!Owner || Children.Num() == 0)
        return EBTNodeStatus_AIC::Failure;

    while (CurrentChildIndex < Children.Num())
    {
        UBTNode_AIC* Child = Children[CurrentChildIndex];
        if (!Child)
            return EBTNodeStatus_AIC::Failure;

        EBTNodeStatus_AIC Status = Child->Execute(Owner);

        if (Status == EBTNodeStatus_AIC::Running)
            return EBTNodeStatus_AIC::Running;

        if (Status == EBTNodeStatus_AIC::Failure)
        {
            Reset();
            return EBTNodeStatus_AIC::Failure;
        }

        CurrentChildIndex++;
    }

    Reset();
    return EBTNodeStatus_AIC::Success;
}

void UBTSequence_AIC::Reset()
{
    CurrentChildIndex = 0;
    for (UBTNode_AIC* Child : Children)
    {
        if (Child)
            Child->Reset();
    }
}

// DECORATOR IMPLEMENTATIONS

EBTNodeStatus_AIC UBTDecorator_AIC::Execute(AAICharacter_BehaviorTree* Owner)
{
    if (!Owner || !Child)
        return EBTNodeStatus_AIC::Failure;

    bool bConditionMet = Condition(Owner);

    if (bConditionMet != ExecuteChildWhenConditionFailed)
    {
        return Child->Execute(Owner);
    }
    else
    {
        return bConditionMet ? EBTNodeStatus_AIC::Success : EBTNodeStatus_AIC::Failure;
    }
}

bool UBTDecorator_IsInChaseRange::Condition(AAICharacter_BehaviorTree* Owner)
{
    if (!Owner)
        return false;
    return Owner->IsTargetInChaseRange();
}

bool UBTDecorator_HasPath::Condition(AAICharacter_BehaviorTree* Owner)
{
    if (!Owner || !Owner->GetPathFollowingSteering())
        return false;

    return Owner->GetPathFollowingSteering()->GetPathPoints().Num() >= 2;
}

bool UBTDecorator_HasReachedDestination::Condition(AAICharacter_BehaviorTree* Owner)
{
    if (!Owner || !::IsValid(Owner->GetPathFollowingSteering()))
        return false;
    return Owner->GetPathFollowingSteering()->HasFinishedPath();
}

EBTNodeStatus_AIC UBTDecorator_WaitTime::Execute(AAICharacter_BehaviorTree* Owner)
{
    if (!Owner)
        return EBTNodeStatus_AIC::Failure;

    if (::IsValid(Child))
    {
        Child->Execute(Owner);
    }

    UWorld* World = Owner->GetWorld();
    if (::IsValid(World))
    {
        Timer += World->GetDeltaSeconds();
    }

    if (Timer >= WaitTime)
    {
        Timer = 0.0f;
        return EBTNodeStatus_AIC::Success;
    }

    bWaiting = true;
    return EBTNodeStatus_AIC::Running;
}

void UBTDecorator_WaitTime::Reset()
{
    bWaiting = false;
    Timer = 0.0f;
    if (Child)
        Child->Reset();
}

// TASK IMPLEMENTATIONS

EBTNodeStatus_AIC UBTTask_WaitTimer::Execute(AAICharacter_BehaviorTree* Owner)
{
    if (!Owner)
        return EBTNodeStatus_AIC::Failure;

    AccumulatedTime += Owner->GetWorld()->GetDeltaSeconds();

    return EBTNodeStatus_AIC::Running;
}

void UBTTask_WaitTimer::Reset()
{
    AccumulatedTime = 0.f;
}

EBTNodeStatus_AIC UBTTask_FindPathToTarget::Execute(AAICharacter_BehaviorTree* Owner)
{
    if (!Owner || !::IsValid(Owner->GetPathFinderGrid()) || !::IsValid(Owner->GetPathFollowingSteering()))
        return EBTNodeStatus_AIC::Failure;

    while (CurrentAttempts < MaxAttempts)
    {
        CurrentAttempts++;

        TArray<FVector> Path = Owner->GetPathFinderGrid()->FindPath(
            Owner->GetActorLocation(),
            Owner->GetTargetActor()->GetActorLocation()
        );

        if (Path.Num() >= 2)
        {
            Owner->GetPathFollowingSteering()->ResetPathFollowingWithPath(Path);

            UE_LOG(LogTemp, Log, TEXT("FindPath: Found valid path with %d waypoints (attempt %d)"),
                Path.Num(), CurrentAttempts);

            Reset();
            return EBTNodeStatus_AIC::Success;
        }

        UE_LOG(LogTemp, Warning, TEXT("FindPath: Invalid path (attempt %d/%d), generating new target"),
            CurrentAttempts, MaxAttempts);
    }

    UE_LOG(LogTemp, Error, TEXT("FindPath: Failed to find valid path after %d attempts"), MaxAttempts);
    Reset();
    return EBTNodeStatus_AIC::Failure;
}

void UBTTask_FindPathToTarget::Reset()
{
    CurrentAttempts = 0;
}

EBTNodeStatus_AIC UBTTask_FindPathToWander::Execute(AAICharacter_BehaviorTree* Owner)
{
    if (!Owner || !::IsValid(Owner->GetPathFinderGrid()) || !::IsValid(Owner->GetPathFollowingSteering()))
        return EBTNodeStatus_AIC::Failure;

    while (CurrentAttempts < MaxAttempts)
    {
        Owner->GenerateWanderTarget();
        CurrentAttempts++;

        TArray<FVector> Path = Owner->GetPathFinderGrid()->FindPath(
            Owner->GetActorLocation(),
            Owner->GetWanderTarget()
        );

        if (Path.Num() >= 2)
        {
            Owner->GetPathFollowingSteering()->ResetPathFollowingWithPath(Path);

            UE_LOG(LogTemp, Log, TEXT("FindPath: Found valid path with %d waypoints (attempt %d)"),
                Path.Num(), CurrentAttempts);

            Reset();
            return EBTNodeStatus_AIC::Success;
        }

        UE_LOG(LogTemp, Warning, TEXT("FindPath: Invalid path (attempt %d/%d), generating new target"),
            CurrentAttempts, MaxAttempts);
    }
    
    UE_LOG(LogTemp, Error, TEXT("FindPath: Failed to find valid path after %d attempts"), MaxAttempts);
    Reset();
    return EBTNodeStatus_AIC::Failure;
}

void UBTTask_FindPathToWander::Reset()
{
    CurrentAttempts = 0;
}

EBTNodeStatus_AIC UBTTask_FollowPath::Execute(AAICharacter_BehaviorTree* Owner)
{
    if (!Owner || !Owner->GetPathFollowingSteering())
        return EBTNodeStatus_AIC::Failure;

    if (Owner->GetPathFollowingSteering()->HasFinishedPath() == true)
    {
        return EBTNodeStatus_AIC::Success;
    }

    return EBTNodeStatus_AIC::Running;
}

EBTNodeStatus_AIC UBTTask_Chase::Execute(AAICharacter_BehaviorTree* Owner)
{
    if (!Owner || !Owner->GetPathFollowingSteering())
        return EBTNodeStatus_AIC::Failure;

    // Regenerate the path.
    TArray<FVector> Path = Owner->GetPathFinderGrid()->FindPath(
        Owner->GetActorLocation(),
        Owner->GetTargetActor()->GetActorLocation()
    );

    if (Path.Num() >= 2)
    {
        Owner->GetPathFollowingSteering()->ResetPathFollowingWithPath(Path);
    }
    else if (Owner->GetPathFollowingSteering()->HasFinishedPath())
    {
        return EBTNodeStatus_AIC::Success;
    }

    return EBTNodeStatus_AIC::Running;
}

// BEHAVIOR TREE IMPLEMENTATION

EBTNodeStatus_AIC UBehaviorTree_AICharacter::Tick(AAICharacter_BehaviorTree* Owner, float DeltaTime)
{
    if (!RootNode)
        return EBTNodeStatus_AIC::Failure;

    return RootNode->Execute(Owner);
}

void UBehaviorTree_AICharacter::Reset()
{
    if (RootNode)
        RootNode->Reset();
}
