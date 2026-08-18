#ifndef __OBSTACLE_H__
#define __OBSTACLE_H__

#include <CoreMinimal.h>

struct ObstacleAttr
{
    FVector Position{ FVector::ZeroVector };
    float Radius{ 0.f };
};

struct FObstacles
{
    TArray<ObstacleAttr> ObstaclesArray;

    TArray<ObstacleAttr> GetObstaclesArray() const { return ObstaclesArray; }
};

bool ReadObstacles(const char* filename, FObstacles& obstacles);

#endif