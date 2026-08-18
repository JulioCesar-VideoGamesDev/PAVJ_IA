#include "SeekSteering.h"
#include "Characters/AICharacter.h"

#include "DrawDebugHelpers.h"

FSteeringOutput USeekSteering::GetSteering()
{
    FSteeringOutput Result;

    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("CHARACTER ISN'T VALID"));

        return Result;
    }

    TargetPosition = Character->GetTargetPosition();

    FVector Direction = Character->GetTargetPosition() - Character->GetActorLocation();

    if (!Direction.IsNearlyZero())
    {
        Direction.Normalize();
    }

    MaxVelocityPosible = Direction * Character->GetParams().max_speed; // Max Velocity posible

    // Subtract CurrentVelocity to MaxVelocityPosible to get the LinearVelocity of the Steering.
    Result.Linear = MaxVelocityPosible - Character->GetCurrentVelocity();
    
    if (Result.Linear.Size() > Character->GetParams().max_acceleration)
    {
        Result.Linear =
            Result.Linear.GetSafeNormal() *
            Character->GetParams().max_acceleration;
    }

    LastAcceleration = Result.Linear;

    Result.Angular = 0.0f;

    DrawDebug(Result);

    return Result;
}

void USeekSteering::DrawDebug(FSteeringOutput Steering)
{
    FVector Start = Character->GetActorLocation();
    FVector End = Start + Character->GetVelocity() + Steering.Linear * 4;

    DrawDebugLine(
        GetWorld(),
        Start,
        End,
        FColor::Red,
        false, // Persistent
        0.f,   // Duration just this frame
        0,     // Depth priority
        8.f   // Thickness
    );
}