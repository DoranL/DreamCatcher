// Fill out your copyright notice in the Description page of Project Settings.


#include "MainAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Nelia.h"

void UMainAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();  //이 애니메이션 인스턴스의 소유자를 가져온다?
		if (Pawn)
		{
			Nelia = Cast<ANelia>(Pawn);
		}
	}
}
void UMainAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
	}

	if (Pawn)
	{
		FVector Speed = Pawn->GetVelocity();
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LateralSpeed.Size();

		bIsInAir = Pawn->GetMovementComponent()->IsFalling();

		if (Nelia == nullptr)
		{
			Nelia = Cast<ANelia>(Pawn);
		}
		RotationValue = Pawn->GetActorRotation().Pitch;
	}
}