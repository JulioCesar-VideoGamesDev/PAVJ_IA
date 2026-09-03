#pragma once

#include "CoreMinimal.h"
#include "Characters/AICharacter.h"
#include "AICharacter_BehaviorTree.generated.h"

class UBehaviorTree_AICharacter;
class UPathFinder_Grid;
class UPathFollowingSteering;
struct FSteeringOutput;

UCLASS()
class MPV_PRACTICAS_API AAICharacter_BehaviorTree : public AAICharacter
{
	GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

    void SetupSteeringBehaviours();
    void SetupBehaviorTree();
    void BuildBehaviorTree();

    virtual void Tick(float DeltaTime) override;

    // Behavior Tree
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI_BehaviorTree|BehaviorTree")
    TObjectPtr<UBehaviorTree_AICharacter> BehaviorTree;

    // Steering Behaviours
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI_BehaviorTree|Steering")
    TObjectPtr<UPathFollowingSteering> PathFollowingSteering;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI_BehaviorTree|Steering")
    TObjectPtr<UPathFinder_Grid> PathFinder_Grid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI_BehaviorTree|Settings")
    float WaitTime{ 0.f };
    // AI Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI_BehaviorTree|Settings")
    float ChaseRange{ 0.f };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI_BehaviorTree|Settings")
    TSubclassOf<AActor> TargetActorClass;
    
    UPROPERTY(BlueprintReadOnly, Category = "AI_BehaviorTree")
    TObjectPtr<AActor> TargetActor{ nullptr };

    // AI State Variables
    UPROPERTY(BlueprintReadOnly, Category = "AI_BehaviorTree|State")
    FVector WanderTarget;

    UFUNCTION(BlueprintCallable, Category = "AI_BehaviorTree")
    void ApplySteering(const FSteeringOutput& Steering, float DeltaTime);

    virtual void DrawDebug() override;

    /*friend class UBTTask_FindPathToTarget;
    friend class UBTDecorator_IsInChaseRange;
    friend class UBTDecorator_HasReachedDestination;*/

public:
    AAICharacter_BehaviorTree();

    // Helper Functions
    UFUNCTION(BlueprintCallable, Category = "AI_BehaviorTree|Helpers")
    void GenerateWanderTarget();

    TObjectPtr<UPathFollowingSteering> GetPathFollowingSteering() const { return PathFollowingSteering; };

    TObjectPtr<UPathFinder_Grid> GetPathFinderGrid() const { return PathFinder_Grid; };

    UFUNCTION(BlueprintCallable, Category = "AI_BehaviorTree|Helpers")
    FVector GetWanderTarget() const { return WanderTarget; };

    UFUNCTION(BlueprintCallable, Category = "AI_BehaviorTree|Helpers")
    float GetDistanceToTargetActor();

    UFUNCTION(BlueprintCallable, Category = "AI_BehaviorTree|Helpers")
    bool IsTargetInChaseRange();

    TObjectPtr<AActor> GetTargetActor() const { return TargetActor; };
};