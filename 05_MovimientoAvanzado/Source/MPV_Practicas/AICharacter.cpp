// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"
#include "debug/debugdraw.h"
#include "SeekSteering.h"

// Sets default values
AAICharacter::AAICharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAICharacter::BeginPlay()
{
	Super::BeginPlay();

	ReadParams("params.xml", m_params);

	ReadPaths("paths.xml", m_paths);

	PathPoints = m_paths.GetPathPoints();

	SeekSteering = NewObject<USeekSteering>(this);
	SeekSteering->Character = this;
}

// Called every frame
void AAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	current_angle = GetActorAngle();

	PathFollowing(); 

	FSteeringOutput Steering;

	if (SeekSteering)
	{
		SeekSteering->TargetPosition = m_params.targetPosition;

		Steering = SeekSteering->GetSteering();
	}

	velocity += Steering.Linear * DeltaTime;

	if (velocity.Length() > m_params.max_velocity)
	{
		velocity =
			velocity.GetSafeNormal() *
			m_params.max_velocity;
	}

	SetActorLocation(
		GetActorLocation() +
		velocity);

	DrawDebug();
}

// Called to bind functionality to input
void AAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAICharacter::OnClickedLeft(const FVector& mousePosition)
{
	SetActorLocation(mousePosition);
}

void AAICharacter::OnClickedRight(const FVector& mousePosition)
{
	m_params.targetPosition = mousePosition;

	FVector dir = (mousePosition - GetActorLocation()).GetSafeNormal();
	float angle = FMath::RadiansToDegrees(atan2(dir.Z, dir.X));
	m_params.targetRotation = angle;
}

void AAICharacter::DrawDebug()
{
	/*TArray<FVector> Points =
	{
		FVector(0.f, 0.f, 0.f),
		FVector(100.f, 0.f, 0.f),
		FVector(100.f, 0.f, 100.f),
		FVector(100.f, 0.f, 100.f),
		FVector(0.f, 0.f, 100.f)
	};*/

	SetPath(this, TEXT("follow_path"), TEXT("path"), PathPoints, 5.0f, PathMaterial);

	SetCircle(this, TEXT("targetPosition"), m_params.targetPosition, 10.0f);
	FVector dir(cos(FMath::DegreesToRadians(m_params.targetRotation)), 0.0f, sin(FMath::DegreesToRadians(m_params.targetRotation)));
	SetArrow(this, TEXT("targetRotation"), dir, 80.0f);

	TArray<TArray<FVector>> Polygons = {
		{ FVector(0.f, 0.f, 0.f), FVector(100.f, 0.f, 0.f), FVector(100.f, 0.f, 100.0f), FVector(0.f, 0.f, 100.0f) },
		{ FVector(100.f, 0.f, 0.f), FVector(200.f, 0.f, 0.f), FVector(200.f, 0.f, 100.0f) }
	};
	SetPolygons(this, TEXT("navmesh"), TEXT("mesh"), Polygons, NavmeshMaterial);

	SetCircle(
		this,
		TEXT("closest"),
		ClosestPoint,
		40.f);

	SetCircle(
		this,
		TEXT("seek"),
		SeekPoint,
		85.f);
}

FVector AAICharacter::GetClosestPointOnSegment(
	const FVector& P,
	const FVector& A,
	const FVector& B)
{
	FVector AB = B - A;

	float LengthSq = FVector::DotProduct(AB, AB);

	if (LengthSq <= KINDA_SMALL_NUMBER)
	{
		return A;
	}

	float t = FVector::DotProduct(P - A, AB) / LengthSq;

	t = FMath::Clamp(t, 0.f, 1.f);

	return A + AB * t;
}

int AAICharacter::GetClosestPathPoint(
	const FVector& Position,
	FVector& OutPoint)
{
	float BestDistance = FLT_MAX;
	int BestSegment = 0;

	for (int i = 0; i < PathPoints.Num() - 1; i++)
	{
		FVector Projection =
			GetClosestPointOnSegment(
				Position,
				PathPoints[i],
				PathPoints[i + 1]);

		float Dist =
			FVector::DistSquared(Position, Projection);

		if (Dist < BestDistance)
		{
			BestDistance = Dist;
			BestSegment = i;
			OutPoint = Projection;
		}
	}

	return BestSegment;
}

FVector AAICharacter::GetLookAheadPoint(
	int Segment,
	const FVector& StartPoint,
	float DistanceAhead)
{
	FVector CurrentPoint = StartPoint;

	int CurrentSegment = Segment;

	while (CurrentSegment < PathPoints.Num() - 1)
	{
		FVector EndPoint =
			PathPoints[CurrentSegment + 1];

		float SegmentLength =
			FVector::Dist(CurrentPoint, EndPoint);

		if (DistanceAhead <= SegmentLength)
		{
			FVector Dir =
				(EndPoint - CurrentPoint).GetSafeNormal();

			return CurrentPoint +
				Dir * DistanceAhead;
		}

		DistanceAhead -= SegmentLength;

		CurrentPoint = EndPoint;

		CurrentSegment++;
	}

	return PathPoints.Last();
}

void AAICharacter::PathFollowing()
{
	FVector Position = GetActorLocation();

	int Segment =
		GetClosestPathPoint(Position, ClosestPoint);

	SeekPoint =
		GetLookAheadPoint(
			Segment,
			ClosestPoint,
			m_params.look_ahead);

	/*UE_LOG(LogTemp, Warning,
		TEXT("Segment=%d  Closest=(%.1f, %.1f)  Seek=(%.1f, %.1f)"),
		Segment,
		ClosestPoint.X,
		ClosestPoint.Z,
		SeekPoint.X,
		SeekPoint.Z);*/

	UE_LOG(LogTemp, Warning,
		TEXT("CurrentSegment %d   DistanceAhead %.2f"),
		Segment,
		m_params.look_ahead);

	UE_LOG(LogTemp, Warning,
		TEXT("SeekPoint %.1f %.1f"),
		SeekPoint.X,
		SeekPoint.Z);

	m_params.targetPosition = SeekPoint;
}