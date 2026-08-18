#include "ArriveSteering.h"

#include "Characters/AICharacter.h"

FSteeringOutput UArriveSteering::GetSteering()
{
    FSteeringOutput Result;

    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("CHARACTER ISN'T VALID"));

        return Result;
    }

    TargetPosition = Character->GetTargetPosition();

    FVector Direction = TargetPosition - Character->GetActorLocation();

    float Distance = Direction.Size();

    if (!Direction.IsNearlyZero())
    {
        Direction.Normalize();
    }

    if (Distance > Character->GetParams().arrive_radius) // Not arriving yet.
    {
        MaxVelocityPosible = Direction * Character->GetParams().max_speed;
    }
    else
    {
        float targetSpeed = (Character->GetParams().max_speed * Distance) / Character->GetParams().arrive_radius;
        MaxVelocityPosible = Direction * targetSpeed;
        UE_LOG(LogTemp, Warning, TEXT("ARRIVING"));
    }

    Result.Linear = MaxVelocityPosible - Character->GetCurrentVelocity();

    // Now that I have my desired velocity, I normalize it and multiply it by my acceleration.
    Result.Linear.Normalize();
    Result.Linear *= Character->GetParams().max_acceleration;

    LastAcceleration = Result.Linear;

    Result.Angular = 0.0f;

    return Result;
}