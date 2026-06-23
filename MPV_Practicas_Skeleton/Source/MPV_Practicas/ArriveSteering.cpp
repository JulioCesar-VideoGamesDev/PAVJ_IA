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

    if (Distance > Character->m_params.arrive_radius) // Not arriving yet.
    {
        LastDesiredVelocity = Direction * Character->m_params.max_velocity;
    }
    else
    {
        float targetSpeed = (Character->m_params.max_velocity * Distance) / Character->m_params.arrive_radius;
        LastDesiredVelocity = Direction * targetSpeed;
        UE_LOG(LogTemp, Warning, TEXT("ARRIVING"));
    }

    Result.Linear = LastDesiredVelocity - Character->GetCurrentVelocity();

    // Now that I have my desired velocity, I normalize it and multiply it by my acceleration.
    Result.Linear.Normalize();
    Result.Linear *= Character->m_params.max_acceleration;

    LastAcceleration = Result.Linear;

    Result.Angular = 0.0f;

    return Result;
}