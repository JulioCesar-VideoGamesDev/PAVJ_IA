#include "SeekSteering.h"
#include "AICharacter.h"

#include "debug/debugdraw.h"

FSteeringOutput USeekSteering::GetSteering()
{
    FSteeringOutput Result;

    if (!Character)
    {
        return Result;
    }

    // a) Velocidad deseada
    LastDesiredVelocity = TargetPosition - Character->GetActorLocation();

    // b) Aceleración necesaria
    Result.Linear = LastDesiredVelocity - Character->GetCurrentVelocity();

    // c) Limitar a máxima aceleración
    Result.Linear.Normalize();
    Result.Linear *= Character->m_params.max_acceleration;

    LastAcceleration = Result.Linear;

    Result.Angular = 0.0f;

    return Result;
}

//void USeekSteering::DrawDebug()
//{
//    if (!Character)
//    {
//        return;
//    }
//
//    //FVector Start = Character->GetActorLocation();
//
//    // Velocidad deseada (verde)
//    SetArrow(
//        Character,
//        TEXT("seek_desired_velocity"),
//        LastDesiredVelocity.GetSafeNormal(),
//        LastDesiredVelocity.Size());
//
//    // Aceleración (rojo)
//    SetArrow(
//        Character,
//        TEXT("seek_acceleration"),
//        LastAcceleration.GetSafeNormal(),
//        LastAcceleration.Size());
//}