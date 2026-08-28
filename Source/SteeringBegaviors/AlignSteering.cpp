#include "AlignSteering.h"
#include "Characters/AICharacter.h"

void UAlignSteering::GetSteering(FSteeringOutput& SteeringOutput)
{
    FSteeringOutput Result;

    if (!::IsValid(AICharacter))
    {
        UE_LOG(LogTemp, Error, TEXT("AICHARACTER ISN'T VALID"));

        SteeringOutput = Result;
        return;
    }

    const float CurrentAngle = AICharacter->GetActorAngle();

    // Angular difference. This also alows us to know if we have to rotate left or right.
    float AngularDifference =
        FMath::FindDeltaAngleDegrees(
            CurrentAngle,
            TargetRotation);

    // Already Aligned.
    if (FMath::Abs(AngularDifference) < KINDA_SMALL_NUMBER)
    {
        Result.Linear = FVector::ZeroVector;
        Result.Angular = AICharacter->GetAICharacterCurrentAngularVelocity() * -1;

        SteeringOutput = Result;
        return;
    }

    if (FMath::Abs(AngularDifference) > AICharacter->GetParams().angular_arrive_angle)
    {
        // Max AngularVelocity posible.
        float MaxAngularVelocity =
            (AngularDifference < 0.f ? -1.f : 1.f) *
            AICharacter->GetParams().max_angular_speed;

        Result.Angular = MaxAngularVelocity - AICharacter->GetAICharacterCurrentAngularVelocity(); // Calculat the acceleration for this iteration.

        if (FMath::Abs(Result.Angular) > AICharacter->GetParams().max_angular_acceleration)
        {
            if (Result.Angular >= 0) Result.Angular = AICharacter->GetParams().max_angular_acceleration;
            else Result.Angular = AICharacter->GetParams().max_angular_acceleration * -1;
            SteeringOutput = Result;
        }
    }
    else // Arriving
    {
        float MaxAngularSpeed = AICharacter->GetParams().max_angular_speed;
        float DesiredAngularSpeed = 0.f;

        DesiredAngularSpeed = MaxAngularSpeed * (AngularDifference / AICharacter->GetParams().angular_arrive_angle);
        //DesiredAngularSpeed = AICharacter->GetAICharacterCurrentAngularVelocity() * AngularDifference / AICharacter->GetParams().angular_arrive_angle;

        DesiredAngularSpeed = FMath::Min(DesiredAngularSpeed, MaxAngularSpeed);

        Result.Angular = DesiredAngularSpeed - AICharacter->GetAICharacterCurrentAngularVelocity();

        // If we have a max_deceleration then we would clamp it here.
    }

    Result.Linear = FVector::ZeroVector;

    SteeringOutput = Result;

    return;
}