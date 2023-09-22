// Fill out your copyright notice in the Description page of Project Settings.


#include "Hypono.h"
#include "AIController.h"
#include "MainPlayerController.h"
#include "Nelia.h"

AHypono::AHypono()
{

}
void AHypono::BeginPlay()
{
	Super::BeginPlay();
}

void AHypono::Attack()
{
	if (Alive() && bHasValidTarget)
	{
		UWorld* world = GetWorld();

		if (AIController)
		{
			AIController->StopMovement();
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
		}

		if (!bAttacking)
		{
			bAttacking = true;
			SetInterpToNelia(true);

			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

			if (AnimInstance && CombatMontage/* && EnemyMovementStatus != EEnemyMovementStatus::EMS_MoveToTarget*/)
			{
				float DistanceToCombat = FVector::Distance(this->GetActorLocation(), Nelia->GetActorLocation());
				int DistanceToCombatInt = FMath::TruncToFloat(DistanceToCombat / 700.f);

				if (DistanceToCombatInt < 3)
				{
					AnimInstance->Montage_Play(CombatMontage, 1.4f);
					AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
				}

			}
		}
	}
}