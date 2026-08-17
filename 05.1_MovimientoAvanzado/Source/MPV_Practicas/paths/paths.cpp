#include "paths.h"

#include "XmlFile.h"
#include "Misc/DefaultValueHelper.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"


bool ReadPaths(const char* filename, Paths& paths)
{
	paths.PathPoints.Empty();

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
			RootNode->FindChildNode(TEXT("points"));

		if (!PointsNode)
			return false;

		const TArray<FXmlNode*>& Children =
			PointsNode->GetChildrenNodes();

		for (const FXmlNode* PointNode : Children)
		{
			if (PointNode->GetTag() == TEXT("point"))
			{
				FString value;

				value = PointNode->GetAttribute(TEXT("x"));
				float x;
				FDefaultValueHelper::ParseFloat(value, x);

				value = PointNode->GetAttribute(TEXT("y"));
				float y;
				FDefaultValueHelper::ParseFloat(value, y);

				paths.PathPoints.Add(
					FVector(x, 0.f, y)
				);
			}
		}

		//const FXmlNode* paramElem = MyChildNode->FindChildNode(TEXT("point"));

		/*if (paramElem)
		{
			value = paramElem->GetAttribute("x");
			float x;
			FDefaultValueHelper::ParseFloat(value, x);
			value = paramElem->GetAttribute("y");
			float z;
			FDefaultValueHelper::ParseFloat(value, z);

			paths.PathPoints.Add(FVector(x, 0.f, z));
		}*/

		/*paramElem = MyChildNode->FindChildNode(TEXT("point_two"));
		if (paramElem)
		{
			value = paramElem->GetAttribute("x");
			float x;
			FDefaultValueHelper::ParseFloat(value, x);
			value = paramElem->GetAttribute("y");
			float z;
			FDefaultValueHelper::ParseFloat(value, z);

			paths.PathPoints.Add(FVector(x, 0.f, z));
		}

		paramElem = MyChildNode->FindChildNode(TEXT("point_three"));
		if (paramElem)
		{
			value = paramElem->GetAttribute("x");
			float x;
			FDefaultValueHelper::ParseFloat(value, x);
			value = paramElem->GetAttribute("z");
			float z;
			FDefaultValueHelper::ParseFloat(value, z);

			paths.PathPoints.Add(FVector(x, 0.f, z));
		}

		paramElem = MyChildNode->FindChildNode(TEXT("point_four"));
		if (paramElem)
		{
			value = paramElem->GetAttribute("x");
			float x;
			FDefaultValueHelper::ParseFloat(value, x);
			value = paramElem->GetAttribute("z");
			float z;
			FDefaultValueHelper::ParseFloat(value, z);

			paths.PathPoints.Add(FVector(x, 0.f, z));
		}*/
		return true;
	}
	else return false;
}