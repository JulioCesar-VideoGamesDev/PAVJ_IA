#ifndef __PARAMS_H__
#define __PARAMS_H__

#include <CoreMinimal.h>

struct Params
{
    // Inital Params
    float initial_speed{ 0.f };
    FVector initial_direction{ FVector::ZeroVector };

    // Linear
    float max_speed{ 0.f };
    float max_acceleration{ 0.f };
    float arrive_radius{ 0.f };
    float brake_min_speed{ 0.f };
    FVector targetPosition{ FVector::ZeroVector };

    // Angular
    float max_angular_speed{ 0.f };
    float max_angular_acceleration{ 0.f };
    float angular_arrive_angle{ 0.f };
    float targetRotation{ 0.f };

    // Path
    float look_ahead{ 0.f };
    float time_ahead{ 0.f };

    // ObstacleAvoidance
    float obstacle_avoidance_strength{ 0.f };
    float obstacle_avoidance_weight{ 0.f };
    float char_radius{ 0.f };
};

bool ReadParams(const char* filename, Params& params);

#endif