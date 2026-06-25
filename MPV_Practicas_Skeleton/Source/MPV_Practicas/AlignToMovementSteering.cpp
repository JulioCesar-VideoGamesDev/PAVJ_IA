#include "AlignToMovementSteering.h"
#include "AlignSteering.h"
#include "AICharacter.h"

FSteeringOutput UAlignToMovementSteering::GetSteering()
{
    if (!Character)
    {
        return FSteeringOutput();
    }

    if (!AlignDelegate)
    {
        AlignDelegate = NewObject<UAlignSteering>(this);
        AlignDelegate->Character = Character;
    }

    const FVector Velocity = Character->GetCurrentVelocity();

    if (Velocity.SizeSquared() < KINDA_SMALL_NUMBER)
    {
        return FSteeringOutput();
    }

    const float TargetAngle =
        FMath::RadiansToDegrees(
            FMath::Atan2(Velocity.Y, Velocity.X)
        );

    AlignDelegate->Character = Character;
    AlignDelegate->TargetRotation = TargetAngle;

    return AlignDelegate->GetSteering();
}