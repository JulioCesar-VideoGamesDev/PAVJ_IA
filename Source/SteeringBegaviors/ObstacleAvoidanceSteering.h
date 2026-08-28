#pragma once

#include "CoreMinimal.h"
#include "Steering.h"
#include "../obstacles/obstacles.h"
#include "ObstacleAvoidanceSteering.generated.h"

class AAICharacter;

USTRUCT(BlueprintType)
struct FObstacleCollisionResult
{
    GENERATED_BODY()

    bool bWillCollide{ false };

    int32 ObstacleIndex{ INDEX_NONE };

    FVector ClosestPoint{ FVector::ZeroVector };
    FVector Difference{ FVector::ZeroVector };

    float Distance{ 0.f };
};

UCLASS()
class MPV_PRACTICAS_API UObstacleAvoidanceSteering : public UObject, public ISteering
{
    GENERATED_BODY()

public:

    // Pointer the the AICharacter used to GetSteering.
    UPROPERTY()
    TObjectPtr<AAICharacter> AICharacter;

    //UPROPERTY()
    TArray<FObstacleAttr> ObstaclesArray;
    
    UFUNCTION(BlueprintCallable, Category = "Streering")
    FSteeringOutput FindCollisionAndGetSteering(const FSteeringOutput& InputSteering);

    UPROPERTY(EditDefaultsOnly, Category = "ObstacleAvoidance")
    float AvoidanceStrength{ 0.f };

    // Rturns if the AI will collide. -_-
    UFUNCTION(BlueprintCallable, Category = "Streering")
    bool WillCollide(FSteeringOutput InputSteering);

    UPROPERTY(BlueprintReadWrite, Category = "ObstacleAvoidance|Debug")
    bool DoDrawDebug{ false };

    UFUNCTION(BlueprintCallable, Category = "Streering")
    FObstacleCollisionResult GetFoundCollision() const { return FoundCollision; }

private:

    UFUNCTION(BlueprintCallable, Category = "Streering")
    FObstacleCollisionResult FindCollision(const FSteeringOutput& InputSteering);

    UPROPERTY(EditDefaultsOnly, Category = "ObstacleAvoidance")
    FObstacleCollisionResult FoundCollision;

    UFUNCTION(BlueprintCallable, Category = "Streering")
    virtual void GetSteering(FSteeringOutput& SteeringOutput) override;

    UFUNCTION(BlueprintCallable, Category = "ObstacleAvoidance|Debug")
    void DrawDebug();
};