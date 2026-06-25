#include "params.h"

#include "XmlFile.h"
#include "Misc/DefaultValueHelper.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"


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

		const FXmlNode* paramElem = MyChildNode->FindChildNode(TEXT("max_velocity"));
		FString value;
		if (paramElem)
		{
			value = paramElem->GetAttribute("value");
			FDefaultValueHelper::ParseFloat(value, params.max_velocity);

		}

		paramElem = MyChildNode->FindChildNode(TEXT("max_acceleration"));
		if (paramElem)
		{
			value = paramElem->GetAttribute("value");
			FDefaultValueHelper::ParseFloat(value, params.max_acceleration);

		}

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
	}
	return true;
}