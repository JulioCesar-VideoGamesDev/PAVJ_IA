#include "ArriveSteering.h"

#include "Characters/AICharacter.h"

void UArriveSteering::GetSteering(FSteeringOutput& SteeringOutput)
{
    FSteeringOutput Result;

    if (!::IsValid(AICharacter))
    {
        UE_LOG(LogTemp, Error, TEXT("AICHARACTER ISN'T VALID"));

        SteeringOutput = Result;
        return;
    }

    FVector Direction = TargetPosition - AICharacter->GetActorLocation();

    float Distance = Direction.Size();

    if (!Direction.IsNearlyZero())
    {
        Direction.Normalize();
    }

    if (Distance > AICharacter->GetParams().arrive_radius) // Not arriving yet.
    {
        Super::GetSteering(Result); // Seek

        SteeringOutput = Result;

        return;
    }
    else // Arriving
    {
        // Ensure that we stop the movement
        if (AICharacter->GetAICharacterCurrentVelocity().Length() < BrakeMinSpeed)
        {
            Result.Linear = AICharacter->GetAICharacterCurrentVelocity() * -1;
        }
        else
        {
            float MaxSpeed = AICharacter->GetParams().max_speed;
            float DesiredSpeed = 0.f;

            DesiredSpeed = MaxSpeed * (Distance / AICharacter->GetParams().arrive_radius);
            //DesiredSpeed = AICharacter->GetAIAICharacterCurrentVelocity().Size() * Distance / AICharacter->GetParams().arrive_radius;

            DesiredSpeed = FMath::Min(DesiredSpeed, MaxSpeed);
            FVector DesiredVelocity = Direction * DesiredSpeed;

            Result.Linear = DesiredVelocity - AICharacter->GetAICharacterCurrentVelocity();

            // If we have a max_deceleration then we would clamp it here.
        }

        Result.Angular = 0.0f;

        if (DoDrawDebug) DrawDebug(Result);

        SteeringOutput = Result;

        return;
    }
}