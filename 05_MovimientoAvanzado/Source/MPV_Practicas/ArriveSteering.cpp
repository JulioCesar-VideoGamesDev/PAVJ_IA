#include "ArriveSteering.h"

#include "AICharacter.h"

FSteeringOutput UArriveSteering::GetSteering()
{
    FSteeringOutput Result;

    if (!Character)
    {
        return Result;
    }

    FVector Direction = TargetPosition - Character->GetActorLocation();

    float Distance = Direction.Size();

    if (!Direction.IsNearlyZero())
    {
        Direction.Normalize();
    }

    if (Distance > Character->GetParams().dest_radius) // Not arriving yet.
    {
        LastDesiredVelocity = Direction * Character->GetParams().max_velocity;
    }
    else
    {
        float targetSpeed = (Character->GetParams().max_velocity * Distance) / Character->GetParams().dest_radius;
        LastDesiredVelocity = Direction * targetSpeed;
        //UE_LOG(LogTemp, Warning, TEXT("ARRIVING"));
    }

    Result.Linear = LastDesiredVelocity - Character->GetCurrentVelocity();

    if (Result.Linear.Size() > Character->GetParams().max_acceleration)
    {
        Result.Linear =
            Result.Linear.GetSafeNormal() *
            Character->GetParams().max_acceleration;
    }

    LastAcceleration = Result.Linear;

    Result.Angular = 0.0f;

    return Result;
}