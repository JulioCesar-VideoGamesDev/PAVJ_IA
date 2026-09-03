#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BehaviorTree_AICharacter.generated.h"

class AAICharacter_BehaviorTree;

UENUM(BlueprintType)
enum class EBTNodeStatus_AIC : uint8
{
    Running,
    Success,
    Failure
};

UCLASS(Blueprintable, BlueprintType, Abstract)
class UBTNode_AIC : public UObject
{
    GENERATED_BODY()

public:
    virtual EBTNodeStatus_AIC Execute(AAICharacter_BehaviorTree* Owner) { return EBTNodeStatus_AIC::Failure; }
    virtual void Reset() {}

    UPROPERTY(BlueprintReadOnly, Category = "BehaviorTree")
    FString NodeName;
};

// COMPOSITE NODES

UCLASS(Blueprintable, BlueprintType)
class UBTComposite_AIC : public UBTNode_AIC
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "BehaviorTree")
    TArray<UBTNode_AIC*> Children;

    virtual void Reset() override
    {
        for (UBTNode_AIC* Child : Children)
        {
            if (Child)
                Child->Reset();
        }
    }
};

// SEQUENCE NODE

UCLASS(Blueprintable, BlueprintType)
class UBTSequence_AIC : public UBTComposite_AIC
{
    GENERATED_BODY()

public:
    UBTSequence_AIC();

    virtual EBTNodeStatus_AIC Execute(AAICharacter_BehaviorTree* Owner) override;
    virtual void Reset() override;

private:
    int32 CurrentChildIndex;
};

// SELECTOR NODE (aka Priority)

UCLASS(Blueprintable, BlueprintType)
class UBTSelector_AIC : public UBTComposite_AIC
{
    GENERATED_BODY()

public:
    UBTSelector_AIC();

    virtual EBTNodeStatus_AIC Execute(AAICharacter_BehaviorTree* Owner) override;
    virtual void Reset() override;

private:
    int32 CurrentChildIndex;
    int32 LastExecutedChildIndex;
};

// DECORATOR NODES

UCLASS(Blueprintable, BlueprintType, Abstract)
class UBTDecorator_AIC : public UBTNode_AIC
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "BehaviorTree")
    UBTNode_AIC* Child;

    // If it's true, it inverts the result.
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyBehaviorTree")
    //bool bInverseCondition = false;
    bool ExecuteChildWhenConditionFailed;

    virtual EBTNodeStatus_AIC Execute(AAICharacter_BehaviorTree* Owner) override;
    virtual bool Condition(AAICharacter_BehaviorTree* Owner) { return true; }
};

UCLASS(Blueprintable, BlueprintType)
class UBTDecorator_IsInChaseRange : public UBTDecorator_AIC
{
    GENERATED_BODY()

public:
    virtual bool Condition(AAICharacter_BehaviorTree* Owner) override;
};

UCLASS(Blueprintable, BlueprintType)
class UBTDecorator_HasPath : public UBTDecorator_AIC
{
    GENERATED_BODY()

public:
    virtual bool Condition(AAICharacter_BehaviorTree* Owner) override;
};

UCLASS(Blueprintable, BlueprintType)
class UBTDecorator_HasReachedDestination : public UBTDecorator_AIC
{
    GENERATED_BODY()

public:
    virtual bool Condition(AAICharacter_BehaviorTree* Owner) override;
};

UCLASS(Blueprintable, BlueprintType)
class UBTDecorator_WaitTime : public UBTDecorator_AIC
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "BehaviorTree")
    float WaitTime = 3.0f;

    virtual EBTNodeStatus_AIC Execute(AAICharacter_BehaviorTree* Owner) override;
    virtual void Reset() override;

private:
    float Timer = 0.0f;
    bool bWaiting = false;
};

// TASK NODES

UCLASS(Blueprintable, BlueprintType)
class UBTTask_AIC : public UBTNode_AIC
{
    GENERATED_BODY()

public:
    virtual EBTNodeStatus_AIC Execute(AAICharacter_BehaviorTree* Owner) override { return EBTNodeStatus_AIC::Success; }
};

UCLASS(Blueprintable, BlueprintType)
class UBTTask_WaitTimer : public UBTTask_AIC
{
    GENERATED_BODY()

public:
    virtual EBTNodeStatus_AIC Execute(AAICharacter_BehaviorTree* Owner) override;
    virtual void Reset() override;

private:
    float AccumulatedTime = 0.0f;
};

UCLASS(Blueprintable, BlueprintType)
class UBTTask_FindPathToWander : public UBTTask_AIC
{
    GENERATED_BODY()

public:
    virtual EBTNodeStatus_AIC Execute(AAICharacter_BehaviorTree* Owner) override;
    virtual void Reset() override;

    // Número máximo de intentos para encontrar un path válido
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyBehaviorTree")
    int32 MaxAttempts = 10;

private:
    int32 CurrentAttempts = 0;
};

UCLASS(Blueprintable, BlueprintType)
class UBTTask_FindPathToTarget : public UBTTask_AIC
{
    GENERATED_BODY()

public:
    virtual EBTNodeStatus_AIC Execute(AAICharacter_BehaviorTree* Owner) override;
    virtual void Reset() override;

    // Número máximo de intentos para encontrar un path válido
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyBehaviorTree")
    int32 MaxAttempts = 10;

private:
    int32 CurrentAttempts = 0;
};

UCLASS(Blueprintable, BlueprintType)
class UBTTask_FollowPath : public UBTTask_AIC
{
    GENERATED_BODY()

public:
    virtual EBTNodeStatus_AIC Execute(AAICharacter_BehaviorTree* Owner) override;
};

UCLASS(Blueprintable, BlueprintType)
class UBTTask_Chase : public UBTTask_AIC
{
    GENERATED_BODY()

public:
    virtual EBTNodeStatus_AIC Execute(AAICharacter_BehaviorTree* Owner) override;
};

// BEHAVIOR TREE ROOT

UCLASS()
class MPV_PRACTICAS_API UBehaviorTree_AICharacter : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "BehaviorTree")
    UBTNode_AIC* RootNode;

    virtual EBTNodeStatus_AIC Tick(AAICharacter_BehaviorTree* Owner, float DeltaTime);
    virtual void Reset();
};
