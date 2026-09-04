#include "StateMachine/States/BaseState.h"
#include "../StateMachine.h"

void UBaseState::OnEnter_Implementation()
{
    TimeInState = 0.f;
    //UE_LOG(LogTemp, Log, TEXT("Entering state: %s"), *Name);
}

void UBaseState::OnExit_Implementation()
{
    TimeInState = 0.f;
    //UE_LOG(LogTemp, Log, TEXT("Exiting state: %s"), *Name);
}

void UBaseState::OnUpdate_Implementation(float DeltaTime)
{
    TimeInState += DeltaTime;

    for (const auto& Transition : Transitions)
    {
        if (Transition.Value())
        {
            OwnerMachine->TransitionTo(Transition.Key);
            return;
        }
    }
}

void UBaseState::AddTransition(const FString& DestinationStateName, TFunction<bool()> ConditionFunction)
{
    Transitions.Add(DestinationStateName, ConditionFunction);
}

bool UBaseState::CanTransition(const FString& DestinationStateName)
{
    if (Transitions.Contains(DestinationStateName))
    {
        return Transitions[DestinationStateName]();
    }
    return false;
}

