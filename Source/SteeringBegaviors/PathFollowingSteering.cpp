#include "PathFollowingSteering.h"
#include "Characters/AICharacter.h"
#include "ArriveSteering.h"
#include "ObstacleAvoidanceSteering.h"

#include "debug/debugdraw.h"

void UPathFollowingSteering::GetSteering(FSteeringOutput& SteeringOutput)
{
    FSteeringOutput ResultPathFollowing;

    if (!::IsValid(AICharacter))
    {
        UE_LOG(LogTemp, Error, TEXT("AICHARACTER ISN'T VALID"));

        SteeringOutput = ResultPathFollowing;
        return;
    }

    if (PathPoints.Num() < 2)
    {
        SteeringOutput = ResultPathFollowing;
        return;
    }

    if (!::IsValid(ArriveDelegate))
    {
        ArriveDelegate = NewObject<UArriveSteering>(this);
        ArriveDelegate->AICharacter = AICharacter;
        ArriveDelegate->DoDrawDebug = false;
    }

    // Shearch for the closest point in the path.

    FClosestPointResult ClosestPoint = GetClosestPoint(AICharacter->GetActorLocation());

    // Advance some distance.

    ArriveDelegate->TargetPosition = AdvanceAlongPath(ClosestPoint, AICharacter->GetParams().look_ahead);

    DrawDebugSphere(
        GetWorld(),
        ArriveDelegate->TargetPosition,
        5.f,
        16,              // Segments
        FColor::Green,
        false,           // Persistent
        0.f              // Duration this frame
    );

    // Then we do seek.
    ArriveDelegate->GetSteering(ResultPathFollowing);

    //UE_LOG(LogTemp, Warning, TEXT("PathFollowingSteering: %s"), *ResultPathFollowing.Linear.ToString());
    //UE_LOG(LogTemp, Warning, TEXT("PathFollowingSteering: %f"), ResultPathFollowing.Linear.Y);

    SteeringOutput = ResultPathFollowing;

    if (!EnableObstacleAvoidance)
    {
        return;
    }

    if (!::IsValid(ObstacleAvoidanceDelegate))
    {
        ObstacleAvoidanceDelegate = NewObject<UObstacleAvoidanceSteering>(this);
        ObstacleAvoidanceDelegate->AICharacter = AICharacter;
        ObstacleAvoidanceDelegate->ObstaclesArray = AICharacter->GetObstaclesArray();
        ObstacleAvoidanceDelegate->AvoidanceStrength = AvoidanceStrength;
    }

    FSteeringOutput ResultObstacleAvoidance;

    ObstacleAvoidanceDelegate->FindCollisionAndGetSteering(
        ResultPathFollowing,
        ResultObstacleAvoidance);

    if (!ObstacleAvoidanceDelegate->GetFoundedCollision().bWillCollide) return;

    //UE_LOG(LogTemp, Warning, TEXT("ObstacleAvoidanceSteering: %s"), *ResultObstacleAvoidance.Linear.ToString());
    //UE_LOG(LogTemp, Warning, TEXT("ObstacleAvoidanceSteering: %f"), ResultObstacleAvoidance.Linear.Y);
    
    SteeringOutput.Linear =
        ResultPathFollowing.Linear +
        ResultObstacleAvoidance.Linear * ObstacleAvoidanceWeight;
    UE_LOG(LogTemp, Warning, TEXT("ObstacleAvoidanceWeight: %f"), ObstacleAvoidanceWeight);

    SteeringOutput.Linear.Normalize();
    SteeringOutput.Linear *= AICharacter->GetParams().max_acceleration;
    
    DrawDebugLine(
        GetWorld(),
        AICharacter->GetActorLocation(),
        AICharacter->GetActorLocation() +
        SteeringOutput.Linear,
        FColor::Green,
        false,
        0.f,
        0,
        5.f
    );

    //UE_LOG(LogTemp, Warning, TEXT("MegedSteerings: %s"), *SteeringOutput.Linear.ToString());
    //UE_LOG(LogTemp, Warning, TEXT("MegedSteerings: %f"), SteeringOutput.Linear.Y);
}

// With this function we go segment by segment trying to get the point that is closest to us.
FClosestPointResult UPathFollowingSteering::GetClosestPoint(const FVector& Position)
{
    FClosestPointResult Result = FClosestPointResult();

    FVector BestPoint = FVector::ZeroVector;
    float BestDistanceSq = TNumericLimits<float>::Max();

    int32 NextSegment = CurrentSegment + 1;

    if (IsLooped)
    {
        NextSegment %= PathPoints.Num();
    }
    else
    {
        NextSegment = FMath::Min(
            NextSegment,
            PathPoints.Num() - 2);
    }

    auto CheckSegment = [&](int32 Segment)
        {
            int32 NextPoint = Segment + 1;

            if (IsLooped)
            {
                NextPoint %= PathPoints.Num();
            }

            FVector Candidate =
                ClosestPointOnSegment(
                    Position,
                    PathPoints[Segment],
                    PathPoints[NextPoint]);

            float DistSq =
                FVector::DistSquared(Position, Candidate);

            if (DistSq < BestDistanceSq)
            {
                BestDistanceSq = DistSq;
                BestPoint = Candidate;
                Result.SegmentIndex = Segment;
            }
        };

    CheckSegment(CurrentSegment);
    CheckSegment(NextSegment);

    Result.Point = BestPoint;

    return Result;
}

FVector UPathFollowingSteering::AdvanceAlongPath(
    const FClosestPointResult& Closest,
    float Distance)
{
    FVector CurrentPoint = Closest.Point;
    int32 Segment = Closest.SegmentIndex;

    while (true) // Travvel all segments.
    {
        int32 NextPointIndex = Segment + 1;

        if (IsLooped)
        {
            NextPointIndex %= PathPoints.Num();
        }
        else if (NextPointIndex >= PathPoints.Num())
        {
            return PathPoints.Last();
        }

        const FVector EndOfSegment = PathPoints[NextPointIndex];

        const float Remaining = FVector::Distance(CurrentPoint, EndOfSegment);

        // The distance remaining fits in this segment.
        if (Distance <= Remaining)
        {
            CurrentSegment = Segment;

            const FVector Direction =
                (EndOfSegment - CurrentPoint).GetSafeNormal();

            return CurrentPoint + Direction * Distance;
        }

        // Consume the remaining distance of this segment.
        Distance -= Remaining;

        // Move to the next segment.
        Segment++;

        if (IsLooped)
        {
            Segment %= PathPoints.Num();
        }
        else if (Segment >= PathPoints.Num() - 1) // We reached the end of the path.
        {
            return PathPoints.Last();
        }
        
        CurrentSegment = Segment;
        CurrentPoint = PathPoints[Segment];
    }
}

void UPathFollowingSteering::DrawPath()
{
    if (PathPoints.Num() == 0) return;

    for (int32 i = 0; i < PathPoints.Num(); i++)
    {
        const int32 NextIndex = (i + 1) % PathPoints.Num();

        DrawDebugLine(
            GetWorld(),
            PathPoints[i],
            PathPoints[NextIndex],
            FColor::Black,
            true
        );
    }
}