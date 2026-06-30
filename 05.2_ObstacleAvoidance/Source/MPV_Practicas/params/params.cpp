#include "params.h"

#include "XmlFile.h"
#include "Misc/DefaultValueHelper.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"

void ReadFloatNode(const FXmlNode* myChildNode, const char* _name, float& destVariable)
{
	FString value;
	const FXmlNode* paramElem;

	FString NameStr(_name);
	paramElem = myChildNode->FindChildNode(*NameStr);
	if (paramElem)
	{
		value = paramElem->GetAttribute("value");
		FDefaultValueHelper::ParseFloat(value, destVariable);

	}
}

bool ReadParams(const char* filename, Params& params)
{
	FString CurrentDirectory = FPlatformProcess::GetCurrentWorkingDirectory();

	// Log or use the current working directory
	UE_LOG(LogTemp, Log, TEXT("Current working directory: %s"), *CurrentDirectory);

	FString ContentFolderDir = FPaths::ProjectContentDir();

	//FString FilePath(TEXT("./params.xml"));
	FString params_path = filename;
	FString FilePath = FPaths::Combine(*ContentFolderDir, *params_path);
	UE_LOG(LogTemp, Log, TEXT("Params Path: %s"), *FilePath);

	
	FXmlFile MyXml(FilePath, EConstructMethod::ConstructFromFile);
	
	if (MyXml.GetRootNode())
	{
		const FXmlNode* RootNode = MyXml.GetRootNode();
	
		const FString MyChildTag("params");
		const FXmlNode* MyChildNode = RootNode->FindChildNode(MyChildTag);

		FString value;
		const FXmlNode* paramElem;

		// LINEAR ------------------------------------------------------------

		ReadFloatNode(MyChildNode, "max_velocity", params.max_velocity);

		ReadFloatNode(MyChildNode, "max_acceleration", params.max_acceleration);

		ReadFloatNode(MyChildNode, "arrive_radius", params.arrive_radius);

		paramElem = MyChildNode->FindChildNode(TEXT("targetPosition"));
		if (paramElem)
		{
			value = paramElem->GetAttribute("x");
			float x;
			FDefaultValueHelper::ParseFloat(value, x);
			params.targetPosition.X = x;
			value = paramElem->GetAttribute("z");
			float z;
			FDefaultValueHelper::ParseFloat(value, z);
			params.targetPosition.Z = z;
			params.targetPosition.Y = 0.0f;
		}

		// ANGULAR ----------------------------------

		ReadFloatNode(MyChildNode, "max_angular_velocity", params.max_angular_velocity);

		ReadFloatNode(MyChildNode, "max_angular_acceleration", params.max_angular_acceleration);

		ReadFloatNode(MyChildNode, "angular_arrive_radius", params.angular_arrive_radius);

		ReadFloatNode(MyChildNode, "targetRotation", params.targetRotation);

		// PATH FOLLOWING ------------------------------------

		ReadFloatNode(MyChildNode, "look_ahead", params.look_ahead);

		ReadFloatNode(MyChildNode, "time_ahead", params.time_ahead);

		// OBSTACLE AVOIDANCE -----------------------------------------

		ReadFloatNode(MyChildNode, "char_radius", params.char_radius);
	}
	return true;
}