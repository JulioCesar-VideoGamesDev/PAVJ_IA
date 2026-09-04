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
        ArriveDelegate->BrakeMinSpeed = AICharacter->GetParams().brake_min_speed;
        ArriveDelegate->DoDrawDebug = false;
    }

    if ((!IsLooped && HasFinishedPath())
        || bHasStopedPathFollowing)
    {
        // If we finished the path and we have an Arrive
        if (::IsValid(ArriveDelegate))
        {
            // We keep getting the arrive steering to stop the movement.
            ArriveDelegate->GetSteering(ResultPathFollowing);
            SteeringOutput = ResultPathFollowing;
            return;
        }
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
        ObstacleAvoidanceDelegate->AvoidanceStrength = ObstacleAvoidanceStrength;
    }

    FSteeringOutput ResultObstacleAvoidance = ObstacleAvoidanceDelegate->FindCollisionAndGetSteering(ResultPathFollowing);

    if (!ObstacleAvoidanceDelegate->GetFoundCollision().bWillCollide) return;
    
    SteeringOutput.Linear =
        ResultPathFollowing.Linear +
        ResultObstacleAvoidance.Linear * ObstacleAvoidanceWeight;

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
}

void UPathFollowingSteering::ResetPathFollowingWithPath(const TArray<FVector>& NewPath, bool bResetPosition)
{
    // Verify if the new path is valid.
    if (NewPath.Num() < 2)
    {
        UE_LOG(LogTemp, Error, TEXT("ResetPathFollowingWithPath: Invalid Path (It's needed at least 2 points in it)"));
        return;
    }
    
    // Clean the last state of all the PathFollowing.
    PathPoints.Empty();
    CurrentSegment = 0;
    bHasStopedPathFollowing = false;

    // Copy the new path.
    PathPoints = NewPath;

    if (!::IsValid(ArriveDelegate))
    {
        UE_LOG(LogTemp, Error, TEXT("ResetPathFollowingWithPath: ArriveDelegate isn't valid when tryed to reset the path"));
        return;
    }

    ArriveDelegate->TargetPosition = PathPoints[0];

    UE_LOG(LogTemp, Log, TEXT("ResetPathFollowingWithPath: Path reseted with %d points. FirstPoint: %s"),
        PathPoints.Num(), *PathPoints[0].ToString());
}

bool UPathFollowingSteering::HasFinishedPath() const
{
    if (IsLooped)
        return false;

    if (PathPoints.Num() < 2)
    {
        UE_LOG(LogTemp, Error, TEXT("Current path is not valid"));
        return true;
    }
    
    if (bHasStopedPathFollowing)
        return true;

    // If we are not in the last segment then we are not even close to finished.
    if (CurrentSegment + 1 < PathPoints.Num())
        return false;

    return AICharacter->GetAICharacterCurrentVelocity().Length() <= KINDA_SMALL_NUMBER;
}

void UPathFollowingSteering::StopPathFollowing()
{
    TArray<FVector> newPath =
    {
        AICharacter->GetActorLocation(),
        AICharacter->GetActorLocation()
    };
    ResetPathFollowingWithPath(newPath);

    bHasStopedPathFollowing = true;

    if (ArriveDelegate)
    {
        ArriveDelegate->TargetPosition = AICharacter->GetActorLocation();
    }

    UE_LOG(LogTemp, Log, TEXT("PathFollowing: Stoped"));
}

void UPathFollowingSteering::ContinuePathFollowing()
{
    bHasStopedPathFollowing = false;

    UE_LOG(LogTemp, Log, TEXT("PathFollowing: Continued"));
}

void UPathFollowingSteering::TogglePathFollowing()
{
    if (bHasStopedPathFollowing) ContinuePathFollowing();
    else StopPathFollowing();
}

// With this function we go segment by segment trying to get the point that is closest to us.
FClosestPointResult UPathFollowingSteering::GetClosestPoint(const FVector& Position)
{
    FClosestPointResult Result = FClosestPointResult();

    if (HasFinishedPath() && PathPoints.Num() > 0)
    {
        Result.Point = PathPoints.Last();
        Result.SegmentIndex = PathPoints.Num() - 2;
        return Result;
    }

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

            if (NextPoint >= PathPoints.Num())
            {
                return;
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
    if (HasFinishedPath() && PathPoints.Num() > 0)
    {
        return PathPoints.Last();
    }

    FVector CurrentPoint = Closest.Point;
    int32 Segment = Closest.SegmentIndex;

    while (IsLooped || Segment < PathPoints.Num() -1) // Travvel all segments.
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

            const FVector Direction = (EndOfSegment - CurrentPoint).GetSafeNormal();

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
            CurrentSegment = Segment;
            return PathPoints.Last();
        }
        
        CurrentSegment = Segment;
        CurrentPoint = PathPoints[Segment];
    }

    return PathPoints.Last();
}

void UPathFollowingSteering::DrawPath()
{
    if (PathPoints.Num() < 2) return;

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