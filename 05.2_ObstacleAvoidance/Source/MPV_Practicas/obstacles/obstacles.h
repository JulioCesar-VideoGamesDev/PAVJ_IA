#ifndef __OBSTACLE_H__
#define __OBSTACLE_H__

#include <CoreMinimal.h>

struct ObstacleAttr
{
    FVector Position;
    float Radius;
};

struct FObstacles
{
    TArray<ObstacleAttr> ObstaclesArray;

    TArray<ObstacleAttr> GetObstaclesArray() { return ObstaclesArray; }
};

bool ReadObstacles(const char* filename, FObstacles& obstacles);

#endif