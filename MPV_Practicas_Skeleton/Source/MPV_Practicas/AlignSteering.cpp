#include "AlignSteering.h"
#include "AICharacter.h"

FSteeringOutput UAlignSteering::GetSteering()
{
    FSteeringOutput Result;

	float fTargetAngleRad = FMath::DegreesToRadians(Character->GetParams().targetRotation);

	float fCurrentAgleRad = FMath::DegreesToRadians(Character->GetActorAngle());

	float fDesiredRotationRad = fTargetAngleRad - fCurrentAgleRad;

	if (fDesiredRotationRad < -PI)
	{
		fDesiredRotationRad = fDesiredRotationRad + (2 * PI);
	}
	else if (fDesiredRotationRad > PI)
	{
		fDesiredRotationRad = fDesiredRotationRad - (2 * PI);
	}

	float fDesiredRotationDeg = FMath::RadiansToDegrees(fDesiredRotationRad);

	if (fDesiredRotationDeg > Character->GetParams().max_angular_velocity)
	{
		fDesiredRotationDeg = Character->GetParams().max_angular_velocity;
	}
	else if (fDesiredRotationDeg < -Character->GetParams().max_angular_velocity)
	{
		fDesiredRotationDeg = -Character->GetParams().max_angular_velocity;
	}

	float fObjetiveAngularVelocity = (fDesiredRotationDeg < 0 ? -1 : 1) * Character->GetParams().max_angular_velocity;

	if (fDesiredRotationDeg < Character->GetParams().angular_arrive_radius)
	{
		float fLimiter = (fDesiredRotationDeg / Character->GetParams().angular_arrive_radius);
		fLimiter = fLimiter < 0 ? -fLimiter : fLimiter;

		float fSteeringMagnitude = Character->GetParams().max_angular_velocity * fLimiter;
		fObjetiveAngularVelocity = (fDesiredRotationDeg < 0 ? -1 : 1) * fSteeringMagnitude;
	}

	Result.Angular = fObjetiveAngularVelocity - Character->GetCurrentAngularVelocity();

    Result.Linear = FVector::Zero();

    return Result;
}
