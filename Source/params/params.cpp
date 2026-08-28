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

		// INITIAL PARAMS ----------------------------------------------------

		ReadFloatNode(MyChildNode, "initial_speed", params.initial_speed);

		paramElem = MyChildNode->FindChildNode(TEXT("initial_direction"));
		if (paramElem)
		{
			value = paramElem->GetAttribute("x");
			float x;
			FDefaultValueHelper::ParseFloat(value, x);
			params.initial_direction.X = x;

			value = paramElem->GetAttribute("y");
			float y;
			FDefaultValueHelper::ParseFloat(value, y);
			params.initial_direction.Y = y;

			value = paramElem->GetAttribute("z");
			float z;
			FDefaultValueHelper::ParseFloat(value, z);
			params.initial_direction.Z = z;
		}

		// LINEAR ------------------------------------------------------------

		ReadFloatNode(MyChildNode, "max_speed", params.max_speed);

		ReadFloatNode(MyChildNode, "max_acceleration", params.max_acceleration);

		ReadFloatNode(MyChildNode, "arrive_radius", params.arrive_radius);

		ReadFloatNode(MyChildNode, "brake_min_speed", params.brake_min_speed);

		paramElem = MyChildNode->FindChildNode(TEXT("targetPosition"));
		if (paramElem)
		{
			value = paramElem->GetAttribute("x");
			float x;
			FDefaultValueHelper::ParseFloat(value, x);
			params.targetPosition.X = x;

			value = paramElem->GetAttribute("y");
			float y;
			FDefaultValueHelper::ParseFloat(value, y);
			params.targetPosition.Y = y;

			value = paramElem->GetAttribute("z");
			float z;
			FDefaultValueHelper::ParseFloat(value, z);
			params.targetPosition.Z = z;
		}

		// ANGULAR ----------------------------------

		ReadFloatNode(MyChildNode, "max_angular_speed", params.max_angular_speed);

		ReadFloatNode(MyChildNode, "max_angular_acceleration", params.max_angular_acceleration);

		ReadFloatNode(MyChildNode, "angular_arrive_angle", params.angular_arrive_angle);

		ReadFloatNode(MyChildNode, "targetRotation", params.targetRotation);

		// PATH FOLLOWING ------------------------------------

		ReadFloatNode(MyChildNode, "look_ahead", params.look_ahead);

		ReadFloatNode(MyChildNode, "time_ahead", params.time_ahead);

		// OBSTACLE AVOIDANCE -------------------------------

		ReadFloatNode(MyChildNode, "obstacle_avoidance_strength", params.obstacle_avoidance_strength);
		ReadFloatNode(MyChildNode, "obstacle_avoidance_weight", params.obstacle_avoidance_weight);
		ReadFloatNode(MyChildNode, "char_radius", params.char_radius);
	}
	return true;
}