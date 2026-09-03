#include "ObstacleAvoidanceSteering.h"
#include "Characters/AICharacter.h"

#include "DrawDebugHelpers.h"

FSteeringOutput UObstacleAvoidanceSteering::FindCollisionAndGetSteering(const FSteeringOutput& InputSteering)
{
    FSteeringOutput Result = FSteeringOutput();

    if (!::IsValid(AICharacter))
    {
        UE_LOG(LogTemp, Error, TEXT("AICHARACTER ISN'T VALID"));

        return Result;
    }

    DrawDebug();

    FoundCollision = FindCollision(InputSteering);
    
    if (FoundCollision.bWillCollide)
    {
        GetSteering(Result);
    }

    return Result;
}

FObstacleCollisionResult UObstacleAvoidanceSteering::FindCollision(const FSteeringOutput& InputSteering)
{
    FObstacleCollisionResult Result;

    if (!::IsValid(AICharacter))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("AICHARACTER ISN'T VALID"));

        return Result;
    }

    const float TimeAhead =
        AICharacter->GetParams().time_ahead;

    const FVector CurrentPosition =
        AICharacter->GetActorLocation();

    const FVector CurrentVelocity =
        AICharacter->GetAICharacterCurrentVelocity();

    const FVector FuturePosition =
        CurrentPosition
        + CurrentVelocity * TimeAhead
        + 0.5f *
        InputSteering.Linear *
        TimeAhead *
        TimeAhead;

    const float CharacterRadius =
        AICharacter->GetParams().char_radius;

    float ClosestDistanceSquared =
        TNumericLimits<float>::Max();

    for (int32 i = 0; i < ObstaclesArray.Num(); ++i)
    {
        const FObstacleAttr& Obstacle =
            ObstaclesArray[i];

        const FVector ClosestPoint =
            ClosestPointOnSegment(
                Obstacle.Position,
                CurrentPosition,
                FuturePosition);

        const float CollisionRadius =
            CharacterRadius +
            Obstacle.Radius;

        const float DistanceSquared =
            FVector::DistSquared(
                ClosestPoint,
                Obstacle.Position);

        if (DistanceSquared <=
            FMath::Square(CollisionRadius))
        {
            if (DistanceSquared <
                ClosestDistanceSquared)
            {
                ClosestDistanceSquared =
                    DistanceSquared;

                Result.bWillCollide = true;
                Result.ObstacleIndex = i;
                Result.ClosestPoint = ClosestPoint;
                Result.Difference =
                    ClosestPoint -
                    Obstacle.Position;
                Result.Distance =
                    FMath::Sqrt(DistanceSquared);
            }
        }
    }

    return Result;
}

void UObstacleAvoidanceSteering::GetSteering(FSteeringOutput& SteeringOutput)
{
    SteeringOutput = FSteeringOutput();

    if (!::IsValid(AICharacter))
    {
        UE_LOG(LogTemp, Error, TEXT("AICHARACTER ISN'T VALID"));

        return;
    }

    if (!FoundCollision.bWillCollide ||
        !ObstaclesArray.IsValidIndex(FoundCollision.ObstacleIndex))
    {
        return;
    }

    const FObstacleAttr& Obstacle =
        ObstaclesArray[FoundCollision.ObstacleIndex];

    const FVector CurrentPosition =
        AICharacter->GetActorLocation();

    const FVector CurrentVelocity =
        AICharacter->GetAICharacterCurrentVelocity();

    const FVector MovementDirection =
        CurrentVelocity.GetSafeNormal();

    if (MovementDirection.IsNearlyZero())
    {
        return;
    }

    // Vector from the obstacle towards the closest point of our predicted trajectory.

    const FVector ToObstacle =
        (Obstacle.Position - CurrentPosition).GetSafeNormal();

    // Get the perpendicular Vector of our direction to the obstacle based on our vertical Axis.
    // This ensures that we don't try to avoid the obstacle by going over it.
    FVector AvoidanceDirection = FVector::CrossProduct(ToObstacle, FVector(0.f, 1.f, 0.f));

    // Determine on which side of our movement direction the obstacle is.
    const float Cross =
        FVector::CrossProduct(
            AvoidanceDirection,
            ToObstacle).Y;

    // We want to move to the opposite side of the obstacle.
    if (Cross < 0.f)
    {
        AvoidanceDirection *= -1.f;
    }

    // Distance that we still need to gain to be outside the collision radius.

    const float CharacterRadius =
        AICharacter->GetParams().char_radius;

    const float CollisionRadius =
        CharacterRadius +
        Obstacle.Radius;

    const float Penetration =
        CollisionRadius -
        FoundCollision.Distance;

    // More penetration = stronger avoidance.

    float AccelerationMagnitude =
        //AICharacter->GetParams().max_acceleration;
        Penetration * AvoidanceStrength;

    AccelerationMagnitude =
        FMath::Clamp(
            AccelerationMagnitude,
            0.f,
            AICharacter->GetParams().max_acceleration);

    //UE_LOG(LogTemp, Warning, TEXT("AccelerationMagnitude: %f"), AccelerationMagnitude);

    SteeringOutput.Linear =
        AvoidanceDirection *
        AccelerationMagnitude;

    DrawDebugLine(
        GetWorld(),
        CurrentPosition,
        CurrentPosition +
        SteeringOutput.Linear,
        FColor::Blue,
        false,
        0.f,
        0,
        5.f
    );

    DrawDebugLine(
        GetWorld(),
        CurrentPosition,
        FoundCollision.ClosestPoint,
        FColor::Yellow,
        false,
        0.f,
        0,
        3.f
    );

    SteeringOutput.Angular = 0.f;
}

bool UObstacleAvoidanceSteering::WillCollide(FSteeringOutput InputSteering)
{
    return FindCollision(InputSteering).bWillCollide;
}

void UObstacleAvoidanceSteering::DrawDebug()
{
    DrawDebugSphere(
        GetWorld(),
        AICharacter->GetActorLocation(),
        AICharacter->GetParams().char_radius,
        16,              // Segments
        FColor::Blue,
        false,           // Persistent
        0.f              // Duration this frame
    );

    for (const FObstacleAttr& Obstacle : ObstaclesArray)
    {
        DrawDebugSphere(
            GetWorld(),
            Obstacle.Position,
            Obstacle.Radius,
            16,              // Segments
            FColor::Red,
            false,           // Persistent
            0.f              // Duration this frame
        );
    }
}