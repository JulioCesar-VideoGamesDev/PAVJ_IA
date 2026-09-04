#include "StateMachine/StateMachine.h"
#include "StateMachine/States/BaseState.h"
#include "Misc/FileHelper.h"
#include "XmlParser.h"

UStateMachine::UStateMachine()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStateMachine::BeginPlay()
{
	Super::BeginPlay();
}

void UStateMachine::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (UpdateWithTickComponent) Update(DeltaTime);
}

bool UStateMachine::LoadFromXML(const FString& FilePath)
{
    FString FileContent;

    // Load configuration from XML
    FString ConfigPath = FPaths::ProjectContentDir() + FilePath;

    if (!FFileHelper::LoadFileToString(FileContent, *ConfigPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load XML file: %s"), *ConfigPath);
        return false;
    }

    return ParseXMLConfiguration(FileContent);
}

bool UStateMachine::ParseXMLConfiguration(const FString& Content)
{
    FXmlFile XMLFile(Content, EConstructMethod::ConstructFromBuffer);

    if (!XMLFile.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse XML file"));
        return false;
    }

    const FXmlNode* RootNode = XMLFile.GetRootNode();
    if (!RootNode || RootNode->GetTag() != TEXT("StateMachine"))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid root node. Expected 'StateMachine'"));
        return false;
    }

    // Process global variables
    const FXmlNode* VariablesNode = RootNode->FindChildNode(TEXT("Variables"));
    if (VariablesNode)
    {
        for (const FXmlNode* VariableNode : VariablesNode->GetChildrenNodes())
        {
            if (VariableNode->GetTag() == TEXT("Variable"))
            {
                FString Name = VariableNode->GetAttribute(TEXT("name"));
                FString Type = VariableNode->GetAttribute(TEXT("type"));
                FString Value = VariableNode->GetAttribute(TEXT("value"));

                if (Type == TEXT("float"))
                {
                    FloatVariables.Add(Name, FCString::Atof(*Value));
                }
                else if (Type == TEXT("bool"))
                {
                    BoolVariables.Add(Name, Value == TEXT("true"));
                }
            }
        }
    }

    // Process states
    const FXmlNode* StatesNode = RootNode->FindChildNode(TEXT("States"));
    if (!StatesNode)
    {
        UE_LOG(LogTemp, Error, TEXT("States node not found"));
        return false;
    }

    for (const FXmlNode* StateNode : StatesNode->GetChildrenNodes())
    {
        if (StateNode->GetTag() == TEXT("State"))
        {
            UBaseState* NewState = NewObject<UBaseState>(this);

            NewState->Name = StateNode->GetAttribute(TEXT("name"));
            NewState->MinTime = FCString::Atof(*StateNode->GetAttribute(TEXT("minTime")));
            NewState->OwnerMachine = this;

            // Process transitions
            const FXmlNode* TransitionsNode = StateNode->FindChildNode(TEXT("Transitions"));
            if (TransitionsNode)
            {
                for (const FXmlNode* TransitionNode : TransitionsNode->GetChildrenNodes())
                {
                    if (TransitionNode->GetTag() == TEXT("Transition"))
                    {
                        FString DestinationState = TransitionNode->GetAttribute(TEXT("destination"));
                        FString ConditionType = TransitionNode->GetAttribute(TEXT("conditionType"));

                        TMap<FString, FString> Parameters;
                        for (const FXmlNode* ParamNode : TransitionNode->GetChildrenNodes())
                        {
                            if (ParamNode->GetTag() == TEXT("Parameter"))
                            {
                                FString ParamName = ParamNode->GetAttribute(TEXT("name"));
                                FString ParamValue = ParamNode->GetAttribute(TEXT("value"));
                                Parameters.Add(ParamName, ParamValue);
                            }
                        }

                        TFunction<bool()> ConditionFunc = CreateCondition(ConditionType, Parameters);
                        NewState->AddTransition(DestinationState, ConditionFunc);
                    }
                }
            }

            RegisterState(NewState);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("State machine loaded successfully. States: %d"), States.Num());
    return true;
}

TFunction<bool()> UStateMachine::CreateCondition(const FString& ConditionType, const TMap<FString, FString>& Parameters)
{
    if (ConditionType == TEXT("FloatVariable"))
    {
        FString VarName = Parameters.FindRef(TEXT("name"));
        FString Operator = Parameters.FindRef(TEXT("operator"));
        float Value = FCString::Atof(*Parameters.FindRef(TEXT("value")));

        return [this, VarName, Operator, Value]() -> bool
            {
                return ConditionFloatVariable(VarName, Operator, Value);
            };
    }
    else if (ConditionType == TEXT("BoolVariable"))
    {
        FString VarName = Parameters.FindRef(TEXT("name"));
        bool Value = Parameters.FindRef(TEXT("value")) == TEXT("true");

        return [this, VarName, Value]() -> bool
            {
                return ConditionBoolVariable(VarName, Value);
            };
    }
    else if (ConditionType == TEXT("TimeInState"))
    {
        float MinTime = FCString::Atof(*Parameters.FindRef(TEXT("min")));

        return [this, MinTime]() -> bool
            {
                return ConditionTimeInState(MinTime);
            };
    }
    else if (ConditionType == TEXT("Always"))
    {
        return []() -> bool { return true; };
    }
    else if (ConditionType == TEXT("Never"))
    {
        return []() -> bool { return false; };
    }

    return []() -> bool { return false; };
}

bool UStateMachine::ConditionFloatVariable(const FString& VariableName, const FString& Operator, float Value)
{
    if (!FloatVariables.Contains(VariableName))
        return false;

    float CurrentValue = FloatVariables[VariableName];

    if (Operator == TEXT(">"))
        return CurrentValue > Value;
    else if (Operator == TEXT(">="))
        return CurrentValue >= Value;
    else if (Operator == TEXT("<"))
        return CurrentValue < Value;
    else if (Operator == TEXT("<="))
        return CurrentValue <= Value;
    else if (Operator == TEXT("=="))
        return CurrentValue == Value;
    else if (Operator == TEXT("!="))
        return CurrentValue != Value;

    return false;
}

bool UStateMachine::ConditionBoolVariable(const FString& VariableName, bool Value)
{
    if (!BoolVariables.Contains(VariableName))
        return false;

    return BoolVariables[VariableName] == Value;
}

bool UStateMachine::ConditionTimeInState(float MinTime)
{
    if (!CurrentState)
        return false;

    return CurrentState->TimeInState >= MinTime;
}

void UStateMachine::RegisterStateCallbacks(const FString& StateName,
    FOnStateEnter OnEnter,
    FOnStateExit OnExit,
    FOnStateUpdate OnUpdate)
{
    if (States.Contains(StateName))
    {
        if (OnEnter.IsBound())
            OnEnterCallbacks.Add(StateName, OnEnter);
        if (OnExit.IsBound())
            OnExitCallbacks.Add(StateName, OnExit);
        if (OnUpdate.IsBound())
            OnUpdateCallbacks.Add(StateName, OnUpdate);
    }
}

void UStateMachine::SetFloatVariable(const FString& VariableName, float Value)
{
    if (FloatVariables.Contains(VariableName))
    {
        FloatVariables[VariableName] = Value;
    }
}

void UStateMachine::SetBoolVariable(const FString& VariableName, bool Value)
{
    if (BoolVariables.Contains(VariableName))
    {
        BoolVariables[VariableName] = Value;
    }
}

float UStateMachine::GetFloatVariable(const FString& VariableName)
{
    if (FloatVariables.Contains(VariableName))
    {
        return FloatVariables[VariableName];
    }
    return 0.f;
}

bool UStateMachine::GetBoolVariable(const FString& VariableName)
{
    if (BoolVariables.Contains(VariableName))
    {
        return BoolVariables[VariableName];
    }
    return false;
}

void UStateMachine::RegisterState(UBaseState* NewState)
{
    if (NewState && !States.Contains(NewState->Name))
    {
        States.Add(NewState->Name, NewState);
        UE_LOG(LogTemp, Log, TEXT("State registered: %s"), *NewState->Name);
    }
}

bool UStateMachine::Start(const FString& InitialState)
{
    if (States.Contains(InitialState))
    {
        CurrentState = States[InitialState];

        // Call Enter callback
        if (OnEnterCallbacks.Contains(InitialState) && OnEnterCallbacks[InitialState].IsBound())
        {
            OnEnterCallbacks[InitialState].Execute();
        }

        CurrentState->OnEnter();
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("Initial state '%s' not found"), *InitialState);
    return false;
}

void UStateMachine::Update(float DeltaTime)
{
    if (!CurrentState)
        return;

    // Call Update callback for current state
    if (OnUpdateCallbacks.Contains(CurrentState->Name) && OnUpdateCallbacks[CurrentState->Name].IsBound())
    {
        OnUpdateCallbacks[CurrentState->Name].Execute(DeltaTime);
    }
    
    // To increment the TimeInState and check the transitions
    CurrentState->OnUpdate(DeltaTime);
}

void UStateMachine::TransitionTo(const FString& NewState)
{
    if (!States.Contains(NewState))
    {
        UE_LOG(LogTemp, Warning, TEXT("State '%s' not found"), *NewState);
        return;
    }

    if (!CurrentState->CanTransition(NewState))
    {
        return;
    }

    // Call Exit callback for current state
    if (OnExitCallbacks.Contains(CurrentState->Name) && OnExitCallbacks[CurrentState->Name].IsBound())
    {
        OnExitCallbacks[CurrentState->Name].Execute();
    }

    // Exit current state
    CurrentState->OnExit();

    // Change state
    FString OldStateName = CurrentState->Name;
    CurrentState = States[NewState];
    CurrentState->Name = NewState;
    CurrentState->TimeInState = 0.0f;

    // Call Enter callback for new state
    if (OnEnterCallbacks.Contains(NewState) && OnEnterCallbacks[NewState].IsBound())
    {
        OnEnterCallbacks[NewState].Execute();
    }

    // Enter new state
    CurrentState->OnEnter();

    UE_LOG(LogTemp, Log, TEXT("Transition: %s -> %s"), *OldStateName, *NewState);
}