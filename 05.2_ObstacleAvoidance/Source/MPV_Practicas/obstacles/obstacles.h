#ifndef __PATH_H__
#define __PATH_H__

#include <CoreMinimal.h>

struct FObstacleAttr
{
    FVector Position;
    float Radius;
};

struct Obstacles
{
    TArray<FObstacleAttr> ObstaclesArray;

    TArray<FObstacleAttr> GetObstaclesArray() { return ObstaclesArray; }
};

bool ReadObstacles(const char* filename, Obstacles& obstacles);

#endif