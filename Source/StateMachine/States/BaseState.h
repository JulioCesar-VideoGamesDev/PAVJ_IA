#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BaseState.generated.h"

class UStateMachine;

UCLASS()
class MPV_PRACTICAS_API UBaseState : public UObject
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State")
    void OnEnter();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State")
    void OnExit();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State")
    void OnUpdate(float DeltaTime);

    void AddTransition(const FString& DestinationStateName, TFunction<bool()> ConditionFunction);

    UFUNCTION(BlueprintCallable, Category = "State")
    bool CanTransition(const FString& DestinationStateName);

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FString Name;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    UStateMachine* OwnerMachine;

    UPROPERTY(BlueprintReadWrite, Category = "State")
    float MinTime;

    UPROPERTY(BlueprintReadWrite, Category = "State")
    float TimeInState;

    TMap<FString, TFunction<bool()>>& GetTransitions() { return Transitions; };

protected:
    TMap<FString, TFunction<bool()>> Transitions;
};
