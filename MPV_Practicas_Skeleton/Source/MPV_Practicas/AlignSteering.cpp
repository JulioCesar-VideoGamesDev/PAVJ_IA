#include "AlignSteering.h"
#include "AICharacter.h"

FSteeringOutput UAlignSteering::GetSteering()
{
    FSteeringOutput Result;

    const float CurrentAngle = Character->GetActorAngle();

    // Diferencia angular mínima [-180,180]
    float DesiredRotation =
        FMath::FindDeltaAngleDegrees(
            CurrentAngle,
            TargetRotation);

    // Ya estamos orientados
    if (FMath::Abs(DesiredRotation) < KINDA_SMALL_NUMBER)
    {
        Result.Linear = FVector::ZeroVector;
        Result.Angular = 0.f;
        return Result;
    }

    float TargetAngularVelocity =
        (DesiredRotation < 0.f ? -1.f : 1.f) *
        Character->GetParams().max_angular_velocity;

    // Frenado al acercarse al objetivo
    if (FMath::Abs(DesiredRotation) <
        Character->GetParams().angular_arrive_radius)
    {
        const float Ratio =
            FMath::Abs(DesiredRotation) /
            Character->GetParams().angular_arrive_radius;

        TargetAngularVelocity *= Ratio;
    }

    // Aceleración angular necesaria
    Result.Angular =
        TargetAngularVelocity -
        Character->GetCurrentAngularVelocity();

    // Limitar aceleración angular
    Result.Angular = FMath::Clamp(
        Result.Angular,
        -Character->GetParams().max_angular_acceleration,
        Character->GetParams().max_angular_acceleration);

    Result.Linear = FVector::ZeroVector;

    return Result;
}