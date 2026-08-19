#include "SeekSteering.h"
#include "Characters/AICharacter.h"

#include "DrawDebugHelpers.h"

void USeekSteering::GetSteering(FSteeringOutput& SteeringOutput)
{
    FSteeringOutput Result;

    if (!::IsValid(AICharacter))
    {
        UE_LOG(LogTemp, Error, TEXT("AICHARACTER ISN'T VALID"));

        SteeringOutput = Result;
        return;
    }

    FVector Direction = TargetPosition - AICharacter->GetActorLocation();

    if (!Direction.IsNearlyZero())
    {
        Direction.Normalize();
    }

    MaxVelocityPosible = Direction * AICharacter->GetParams().max_speed; // Max Velocity posible

    // Subtract CurrentVelocity to MaxVelocityPosible to get the LinearVelocity of the Steering.
    Result.Linear = MaxVelocityPosible - AICharacter->GetAICharacterCurrentVelocity();

    if (Result.Linear.Size() > AICharacter->GetParams().max_acceleration)
    {
        Result.Linear =
            Result.Linear.GetSafeNormal() *
            AICharacter->GetParams().max_acceleration;
    }

    LastAcceleration = Result.Linear;

    Result.Angular = 0.0f;

    if (DoDrawDebug) DrawDebug(Result);

    SteeringOutput = Result;
}

void USeekSteering::DrawDebug(FSteeringOutput Steering)
{
    FVector Start = AICharacter->GetActorLocation();
    FVector End = Start + AICharacter->GetAICharacterCurrentVelocity() + Steering.Linear;

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