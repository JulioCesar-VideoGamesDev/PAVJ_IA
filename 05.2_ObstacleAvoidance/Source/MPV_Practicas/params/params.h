#ifndef __PARAMS_H__
#define __PARAMS_H__

#include <CoreMinimal.h>

struct Params
{
    // Linear
    FVector targetPosition{ 0.f, 0.f, 0.f };
    float max_velocity{ 0.f };
    float max_acceleration{ 0.f };
    float arrive_radius{ 0.f };

    // Angular
    float targetRotation{ 0.f };
    float max_angular_velocity{ 0.f };
    float max_angular_acceleration{ 0.f };
    float angular_arrive_radius{ 0.f };

    // Path
    float look_ahead{ 0.f };
    float time_ahead{ 0.f };

    // Obstacle Avoidance
    float char_radius{ 0.f };
};

bool ReadParams(const char* filename, Params& params);

#endif