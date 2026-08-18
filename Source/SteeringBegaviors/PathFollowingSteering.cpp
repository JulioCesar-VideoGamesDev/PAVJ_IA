#include "PathFollowingSteering.h"
#include "Characters/AICharacter.h"
#include "SeekSteering.h"

#include "debug/debugdraw.h"

FSteeringOutput UPathFollowingSteering::GetSteering()
{
    FSteeringOutput Result;

    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("CHARACTER ISN'T VALID"));

        return Result;
    }

    if (!SeekDelegate)
    {
        SeekDelegate = NewObject<USeekSteering>(this);
        SeekDelegate->Character = Character;
    }

    // Shearch for the closest point in the path.

    FClosestPointResult ClosestPoint = GetClosestPoint(Character->GetActorLocation());

    // Advance some distance.

    SeekDelegate->TargetPosition = AdvanceAlongPath(ClosestPoint, Character->GetParams().look_ahead);

    // Then we do seek.
    Result = SeekDelegate->GetSteering();

    return Result;
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
    float BestDistance = TNumericLimits<float>::Max();

    for (int32 i = 0; i < PathPoints.Num() - 1; ++i)
    {
        FVector Candidate =
            ClosestPointOnSegment(
                Position,
                PathPoints[i],
                PathPoints[i + 1]);

        float DistSq =
            FVector::DistSquared(Position, Candidate);

        if (DistSq < BestDistance)
        {
            BestDistance = DistSq;
            BestPoint = Candidate;

            Result.SegmentIndex = i;
        }
    }

    Result.Point = BestPoint;

    return Result;
}

FVector UPathFollowingSteering::AdvanceAlongPath(
    const FClosestPointResult& Closest,
    float Distance)
{
    FVector Current = Closest.Point;

    int32 Segment = Closest.SegmentIndex;

    while (Segment < PathPoints.Num() - 1)
    {
        FVector End = PathPoints[Segment + 1];

        float Remaining =
            FVector::Distance(Current, End);

        if (Distance <= Remaining)
        {
            FVector Direction =
                (End - Current).GetSafeNormal();

            return Current + Direction * Distance;
        }

        Distance -= Remaining;

        Segment++;

        Current = PathPoints[Segment];
    }

    return PathPoints.Last();
}