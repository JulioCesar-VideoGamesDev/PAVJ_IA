#ifndef __PARAMS_H__
#define __PARAMS_H__

#include <CoreMinimal.h>

struct Params
{
    // Linear
    FVector targetPosition;
    float max_velocity;
    float max_acceleration;
    float dest_radius;

    // Angular
    float targetRotation;
    float max_angular_velocity;
    float max_angular_acceleration;
    float angular_arrive_radius;

    // Path
    float look_ahead;
    float time_ahead;
};

bool ReadParams(const char* filename, Params& params);

#endif