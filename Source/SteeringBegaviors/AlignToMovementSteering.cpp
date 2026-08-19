#include "AlignToMovementSteering.h"
#include "Characters/AICharacter.h"

#include "AlignSteering.h"

void UAlignToMovementSteering::GetSteering(FSteeringOutput& SteeringOutput)
{
    FSteeringOutput Result;

    if (!::IsValid(AICharacter))
    {
        UE_LOG(LogTemp, Error, TEXT("AICHARACTER ISN'T VALID"));

        SteeringOutput = Result;
        return;
    }

    if (!::IsValid(AlignDelegate))
    {
        AlignDelegate = NewObject<UAlignSteering>(this);
        AlignDelegate->AICharacter = AICharacter;
    }

    const FVector Velocity = AICharacter->GetAICharacterCurrentVelocity();
    
    // We don't have a movement direction.
    if (Velocity.IsNearlyZero())
    {
        Result.Linear = FVector::ZeroVector;
        Result.Angular = 0.f;

        SteeringOutput = Result;
        return;
    }

    // The direction we are moving towards is our target rotation.
    // This will cause that the AI aligns backwards if the velocity is oposite to the direction that its facing.
    // const float TargetPitch = Velocity.Rotation().Pitch;
    
    // To ensure that we rotate to face the direction that we are moving.
    const float TargetPitch =
        FMath::RadiansToDegrees(
            FMath::Atan2(Velocity.Z, Velocity.X)
        );
    
    AlignDelegate->TargetRotation = TargetPitch;

    // Let AlignSteering calculate the angular acceleration.
    AlignDelegate->GetSteering(Result);

    SteeringOutput = Result;
}