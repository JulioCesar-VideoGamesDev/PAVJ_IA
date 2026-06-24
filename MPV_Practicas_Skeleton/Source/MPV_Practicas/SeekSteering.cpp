#include "SeekSteering.h"
#include "AICharacter.h"

#include "debug/debugdraw.h"

FSteeringOutput USeekSteering::GetSteering()
{
    FSteeringOutput Result;

    if (!Character)
    {
        return Result;
    }

    FVector Direction = TargetPosition - Character->GetActorLocation();

    if (!Direction.IsNearlyZero())
    {
        Direction.Normalize();
    }

    LastDesiredVelocity = Direction * Character->GetParams().max_velocity;

    Result.Linear = LastDesiredVelocity - Character->GetCurrentVelocity();
    
    // Now that I have my desired velocity, I normalize it and multiply it by my acceleration.
    Result.Linear.Normalize();
    Result.Linear *= Character->GetParams().max_acceleration;

    LastAcceleration = Result.Linear;

    Result.Angular = 0.0f;

    return Result;
}

void USeekSteering::DrawDebug()
{
    if (!Character)
    {
        return;
    }
    
    //void SetArrow(const AActor * owner, const FString & arrow_name, const FVector & direction, float length)
    
    // Red
    SetArrow(
        Character,
        TEXT("linear_velocity"),
        LastDesiredVelocity.GetSafeNormal(),
        LastDesiredVelocity.Length()
    );

    // Blue
    SetArrow(
        Character,
        TEXT("linear_acceleration"),
        LastAcceleration.GetSafeNormal(),
        LastAcceleration.Length()
    );
}