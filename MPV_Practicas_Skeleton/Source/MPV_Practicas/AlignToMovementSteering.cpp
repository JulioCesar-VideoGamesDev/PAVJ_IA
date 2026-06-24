#include "AlignToMovementSteering.h"
#include "AICharacter.h"

FSteeringOutput UAlignToMovementSteering::GetSteering()
{
	FSteeringOutput Result;

	FVector Velocity = Character->GetVelocity();

	if (Velocity.SizeSquared() < KINDA_SMALL_NUMBER)
	{
		return FSteeringOutput();
	}

	float TargetAngle =
		FMath::RadiansToDegrees(
			FMath::Atan2(Velocity.Y, Velocity.X)
		);

	//Character->GetParams().targetRotation = TargetAngle;

	return AlignDelegate->GetSteering();
}
