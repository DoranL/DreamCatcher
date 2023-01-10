// Fill out your copyright notice in the Description page of Project Settings.


#include "FollwerAnimInstance.h"
#include "FollowCharacter.h"


void UFollwerAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			FollowerCharacter = Cast<AFollowCharacter>(Pawn);
		}
	}
}

void UFollwerAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			FollowerCharacter = Cast<AFollowCharacter>(Pawn);
		}
	}
	if (Pawn)
	{
		FVector Speed = Pawn->GetVelocity();
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LateralSpeed.Size();
	}
}
