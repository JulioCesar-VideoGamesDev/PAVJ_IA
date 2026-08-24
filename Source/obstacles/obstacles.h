#ifndef __OBSTACLE_H__
#define __OBSTACLE_H__

#include <CoreMinimal.h>

struct FObstacleAttr
{
    FVector Position{ FVector::ZeroVector };
    float Radius{ 0.f };
};

bool ReadObstacles(const char* filename, TArray<FObstacleAttr>& obstaclesArray);

#endif