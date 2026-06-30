#include "obstacles.h"

#include "XmlFile.h"
#include "Misc/DefaultValueHelper.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"


bool ReadObstacles(const char* filename, Obstacles& obstacles)
{
	obstacles.ObstaclesArray.Empty();

	FString CurrentDirectory = FPlatformProcess::GetCurrentWorkingDirectory();

	// Log or use the current working directory
	UE_LOG(LogTemp, Log, TEXT("Current working directory: %s"), *CurrentDirectory);

	FString ContentFolderDir = FPaths::ProjectContentDir();

	//FString FilePath(TEXT("./params.xml"));
	FString params_path = filename;
	FString FilePath = FPaths::Combine(*ContentFolderDir, *params_path);
	UE_LOG(LogTemp, Log, TEXT("Paths Path: %s"), *FilePath);


	FXmlFile MyXml(FilePath, EConstructMethod::ConstructFromFile);

	if (MyXml.GetRootNode())
	{
		const FXmlNode* RootNode =
			MyXml.GetRootNode();

		const FXmlNode* PointsNode =
			RootNode->FindChildNode(TEXT("obstacles"));

		if (!PointsNode)
			return false;

		const TArray<FXmlNode*>& Children =
			PointsNode->GetChildrenNodes();

		for (const FXmlNode* PointNode : Children)
		{
			if (PointNode->GetTag() == TEXT("obstacle"))
			{
				FString value;

				value = PointNode->GetAttribute(TEXT("x"));
				float x;
				FDefaultValueHelper::ParseFloat(value, x);

				value = PointNode->GetAttribute(TEXT("y"));
				float y;
				FDefaultValueHelper::ParseFloat(value, y);

				value = PointNode->GetAttribute(TEXT("r"));
				float r;
				FDefaultValueHelper::ParseFloat(value, r);

				obstacles.ObstaclesArray.Add(
					FObstacleAttr(FVector(x, 0.f, y), r)					
				);
			}
		}

		return true;
	}
	else return false;
}