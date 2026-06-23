#ifndef __PARAMS_H__
#define __PARAMS_H__

#include <CoreMinimal.h>

struct Params
{
    // Linear
    float initial_velocity;

    FVector targetPosition;

    float max_velocity;
    float max_acceleration;

    float arrive_radius;

    // Angular
    float targetRotation;

    float max_angular_velocity;
    float max_angular_acceleration;

    float angular_arrive_radius;
};

bool ReadParams(const char* filename, Params& params);

#endif