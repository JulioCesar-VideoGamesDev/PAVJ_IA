#pragma once

#include "CoreMinimal.h"
#include "Characters/AICharacter.h"
#include "AICharacter_StateMachine.generated.h"

class UStateMachine;

class UPathFinder_Grid;

class UPathFollowingSteering;
struct FSteeringOutput;

UCLASS()
class MPV_PRACTICAS_API AAICharacter_StateMachine : public AAICharacter
{
	GENERATED_BODY()
	
public:
	AAICharacter_StateMachine();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void SetupSteeringBehaviours();
	virtual void SetupStateMachine();

	virtual void RegisterStateCallbacks();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// -------------------------------------------

public:

	// State Machine
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI_StateMachine")
	TObjectPtr<UStateMachine> StateMachine{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI_StateMachine|Steering")
	TObjectPtr<UPathFollowingSteering> PathFollowingSteering{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI_StateMachine|Steering")
	bool ReachedTargetLocation{ false };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI_StateMachine|Steering")
	TObjectPtr<UPathFinder_Grid> PathFinder_Grid{ nullptr };

	// -------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "AI_StateMachine")
	TObjectPtr<AActor> TargetActor{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI_StateMachine")
	TSubclassOf<AActor> TargetActorClass;

	// -------------------------------------------
	
	// AI State variables
	UPROPERTY(BlueprintReadOnly, Category = "AI_StateMachine")
	float IdleTimer{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI_StateMachine")
	float IdleDuration{ 0.f };

	UPROPERTY(BlueprintReadOnly, Category = "AI_StateMachine")
	FVector WanderTarget{ FVector::ZeroVector };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI_StateMachine")
	float ChaseRange{ 0.f };

	// -------------------------------------------

	// AI State functions
	UFUNCTION(BlueprintCallable, Category = "AI_StateMachine")
	void OnEnterIdle();

	UFUNCTION(BlueprintCallable, Category = "AI_StateMachine")
	void OnUpdateIdle(const float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "AI_StateMachine")
	void OnExitIdle();

	UFUNCTION(BlueprintCallable, Category = "AI_StateMachine")
	void OnEnterWander();

	UFUNCTION(BlueprintCallable, Category = "AI_StateMachine")
	void OnUpdateWander(const float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "AI_StateMachine")
	void OnExitWander();

	UFUNCTION(BlueprintCallable, Category = "AI_StateMachine")
	void OnEnterChase();

	UFUNCTION(BlueprintCallable, Category = "AI_StateMachine")
	void OnUpdateChase(const float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "AI_StateMachine")
	void OnExitChase();

	// Helper functions
	UFUNCTION(BlueprintCallable, Category = "AI_StateMachine")
	void GenerateWanderTarget();

	UFUNCTION(BlueprintCallable, Category = "AI_StateMachine")
	float GetDistanceToTargetActor();

	UFUNCTION(BlueprintCallable, Category = "AI_StateMachine")
	bool IsTargetActorInChaseRange();

	UFUNCTION(BlueprintCallable, Category = "AI_StateMachine")
	void ApplySteering(const FSteeringOutput& Steering, const float DeltaTime);
};
