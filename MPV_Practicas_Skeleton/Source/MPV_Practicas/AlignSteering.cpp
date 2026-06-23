#include "AlignSteering.h"
#include "AICharacter.h"

FSteeringOutput UAlignSteering::GetSteering()
{
    FSteeringOutput Result;

    float CurrentRotation = Character->GetActorRotation().Yaw;

    float RotationDifference =
        FMath::FindDeltaAngleDegrees(
            CurrentRotation,
            Character->m_params.targetRotation
        );

    float RotationSize = FMath::Abs(RotationDifference);

    float DesiredAngularVelocity;

    if (RotationSize > Character->m_params.angular_arrive_radius)
    {
        DesiredAngularVelocity =
            FMath::Sign(RotationDifference) *
            Character->m_params.max_angular_velocity;
    }
    else
    {
        DesiredAngularVelocity =
            FMath::Sign(RotationDifference) *
            Character->m_params.max_angular_velocity *
            RotationSize /
            Character->m_params.angular_arrive_radius;
    }

    Result.Angular =
        DesiredAngularVelocity -
        Character->GetCurrentAngularVelocity();

    Result.Angular =
        FMath::Clamp(
            Result.Angular,
            -Character->m_params.max_angular_acceleration,
            Character->m_params.max_angular_acceleration
        );
    Result.Linear = FVector::Zero();

    return Result;
}
