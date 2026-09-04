#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StateMachine.generated.h"

class UBaseState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPV_PRACTICAS_API UStateMachine : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStateMachine();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    bool LoadFromXML(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    bool Start(const FString& InitialState);

    UPROPERTY(EditDefaultsOnly, Category = "StateMachine")
    bool UpdateWithTickComponent{ false };

    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void Update(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void TransitionTo(const FString& NewState);

    // State callbacks - these will be called from AICharacter
    DECLARE_DELEGATE(FOnStateEnter);
    DECLARE_DELEGATE(FOnStateExit);
    DECLARE_DELEGATE_OneParam(FOnStateUpdate, float);

    // Register callbacks for a specific state
    //UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void RegisterStateCallbacks(const FString& StateName,
        FOnStateEnter OnEnter,
        FOnStateExit OnExit,
        FOnStateUpdate OnUpdate);

    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void SetFloatVariable(const FString& VariableName, float Value);

    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void SetBoolVariable(const FString& VariableName, bool Value);

    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    float GetFloatVariable(const FString& VariableName);

    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    bool GetBoolVariable(const FString& VariableName);

    UPROPERTY(BlueprintReadOnly, Category = "StateMachine")
    TObjectPtr<UBaseState> CurrentState;

protected:
    UPROPERTY()
    TMap<FString, UBaseState*> States;

    TMap<FString, float> FloatVariables;
    TMap<FString, bool> BoolVariables;

    // State callbacks
    TMap<FString, FOnStateEnter> OnEnterCallbacks;
    TMap<FString, FOnStateExit> OnExitCallbacks;
    TMap<FString, FOnStateUpdate> OnUpdateCallbacks;

    void RegisterState(UBaseState* NewState);
    bool ParseXMLConfiguration(const FString& Content);

    TFunction<bool()> CreateCondition(const FString& ConditionType, const TMap<FString, FString>& Parameters);

    bool ConditionFloatVariable(const FString& VariableName, const FString& Operator, float Value);
    bool ConditionBoolVariable(const FString& VariableName, bool Value);
    bool ConditionTimeInState(float MinTime);		
};
