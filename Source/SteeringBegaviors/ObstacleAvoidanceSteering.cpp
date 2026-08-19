#include "ObstacleAvoidanceSteering.h"
#include "Characters/AICharacter.h"

#include "debug/debugdraw.h"

void UObstacleAvoidanceSteering::GetSteering(FSteeringOutput& SteeringOutput)
{
    FSteeringOutput Result;

    if (!::IsValid(AICharacter))
    {
        UE_LOG(LogTemp, Error, TEXT("AICHARACTER ISN'T VALID"));

        SteeringOutput = Result;
        return;
    }

    FVector Position = AICharacter->GetActorLocation();
    FVector Velocity = AICharacter->GetAICharacterCurrentVelocity();

    if (Velocity.IsNearlyZero())
    {
        SteeringOutput = Result;
    }

    FVector Direction = Velocity.GetSafeNormal();

    const float LookAhead = 100.f;
    const float CharacterRadius = AICharacter->GetParams().char_radius;

    float BestProjection = TNumericLimits<float>::Max();
    bool bCollision = false;

    FVector AvoidDirection;

    for (const ObstacleAttr& Obstacle : ObstaclesStruct.ObstaclesArray)
    {
        FVector RO = Obstacle.Position - Position;

        float Projection =
            FVector::DotProduct(RO, Direction);

        if (Projection < 0.f)
            continue;

        if (Projection > LookAhead)
            continue;

        FVector ClosestPoint =
            Position +
            Direction * Projection;

        FVector Diff =
            ClosestPoint -
            Obstacle.Position;

        float Distance = Diff.Length();

        if (Distance < Obstacle.Radius + CharacterRadius)
        {
            if (Projection < BestProjection)
            {
                BestProjection = Projection;
                bCollision = true;

                AvoidDirection = Diff.GetSafeNormal();
            }
        }
    }

    if (!bCollision)
    {
        Result.Linear = FVector::ZeroVector;
        Result.Angular = 0.f;
        SteeringOutput = Result;
    }

    Result.Linear =
        AvoidDirection *
        AICharacter->GetParams().max_acceleration;

    Result.Angular = 0.f;

    SteeringOutput = Result;
}