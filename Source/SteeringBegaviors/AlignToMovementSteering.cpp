#include "AlignToMovementSteering.h"
#include "Characters/AICharacter.h"

#include "AlignSteering.h"

FSteeringOutput UAlignToMovementSteering::GetSteering()
{
    FSteeringOutput Result;

    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("CHARACTER ISN'T VALID"));

        return Result;
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

    Result = AlignDelegate->GetSteering();
    return Result;
}