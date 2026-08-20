#include "PathFollowingSteering.h"
#include "Characters/AICharacter.h"
#include "ArriveSteering.h"

#include "debug/debugdraw.h"

void UPathFollowingSteering::GetSteering(FSteeringOutput& SteeringOutput)
{
    FSteeringOutput Result;

    if (!::IsValid(AICharacter))
    {
        UE_LOG(LogTemp, Error, TEXT("AICHARACTER ISN'T VALID"));

        SteeringOutput = Result;
        return;
    }

    if (PathPoints.Num() < 2)
    {
        SteeringOutput = Result;
        return;
    }

    if (!::IsValid(ArriveDelegate))
    {
        ArriveDelegate = NewObject<UArriveSteering>(this);
        ArriveDelegate->AICharacter = AICharacter;
    }

    // Shearch for the closest point in the path.

    FClosestPointResult ClosestPoint = GetClosestPoint(AICharacter->GetActorLocation());

    // Advance some distance.

    ArriveDelegate->TargetPosition = AdvanceAlongPath(ClosestPoint, AICharacter->GetParams().look_ahead);

    // Then we do seek.
    ArriveDelegate->GetSteering(Result);

    SteeringOutput = Result;
}

FVector UPathFollowingSteering::ClosestPointOnSegment(
    const FVector& Point,
    const FVector& A,
    const FVector& B)
{
    FVector AB = B - A;

    float LengthSquared = AB.SizeSquared();

    if (LengthSquared <= KINDA_SMALL_NUMBER)
        return A;

    float T = FVector::DotProduct(Point - A, AB) / LengthSquared;

    T = FMath::Clamp(T, 0.f, 1.f);

    return A + AB * T;
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