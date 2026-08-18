#ifndef __PATH_H__
#define __PATH_H__

#include <CoreMinimal.h>

struct Paths
{
    TArray<FVector> PathPoints;

    TArray<FVector> GetPathPoints() const { return PathPoints; }
};

bool ReadPaths(const char* filename, Paths& paths);

#endif