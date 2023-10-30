// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss.h"
#include "AIController.h"
#include "MainPlayerController.h"
#include "Nelia.h"
#include "Kismet/GameplayStatics.h"

ABoss::ABoss()
{
	//Super();
}

void ABoss::BeginPlay()
{
	Super::BeginPlay();

	bGetup = false;
	countHit = 0;
	timerValue = 3.f;
} 

void ABoss::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive())
	{
		ANelia* Target = Cast<ANelia>(OtherActor);

		//2023-6-24 if 조건문에 !bAttacking 추가함 - 캐릭터가 공격 중에 target쪽으로 이동하는 문제 해결을 목적으로 함 
		if (Target)
		{
			MoveToTarget(Target);
			bOverlappingAgroSphere = true;
			MainPlayerController->DisplayEnemyHealthBar();
		}
	}
}

float ABoss::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	//2023-07-23 수정사항
	//MainPlayerController = Cast<AMainPlayerController>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetController());

	if (GetWorld())
	{
		MainPlayerController = Cast<AMainPlayerController>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetController());
	}
	if (Nelia->bTargeting || Nelia->bHasCombatTarget)
	{
		if (Health - DamageAmount <= 0.f)
		{
			Health -= DamageAmount;
			Die(DamageCauser);
			bTakeDamage = false;
			//MainPlayerController->RemoveEnemyHealthBar();
		}
		else
		{
			bTakeDamage = true;

			MainPlayerController->DisplayEnemyHealthBar();

			Health -= DamageAmount;
			
			if (!bGetup)
			{
				if (AnimInstance)
				{
					countHit++;

					if (countHit > 5)
					{
						AnimInstance->Montage_Play(CombatMontage, 0.8f);
						AnimInstance->Montage_JumpToSection(FName("Recover"), CombatMontage);
						countHit = 0;
						bGetup = true; // Set bGetup to true to prevent further attacks during recovery

						GetWorldTimerManager().SetTimer(AttackTimer, this, &ABoss::GetUpEnd, timerValue); 
					
						/*GetWorld()->GetTimerManager().SetTimer(AttackTimer, FTimerDelegate::CreateLambda([&]()
							{
								GetUpEnd();
								GetWorld()->GetTimerManager().ClearTimer(AttackTimer);
								UE_LOG(LogTemp, Warning, TEXT("In Timer"));
							}), timerValue, false);*/

						//UE_LOG(LogTemp, Warning, TEXT("TimerCountDown %f"), timerValue);
					}
					else
					{
						AnimInstance->Montage_Play(CombatMontage, 1.f);
						AnimInstance->Montage_JumpToSection(FName("Hit"), CombatMontage);
					}
				}
			}
		}
	}

	return DamageAmount;
}

void ABoss::GetUpEnd()
{
	bGetup = false;
}


void ABoss::Attack()
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
			if (AnimInstance && CombatMontage && bInterpToNelia/* && EnemyMovementStatus != EEnemyMovementStatus::EMS_MoveToTarget*/)
			{
				int countAttack = EnemyAttackCounting;

				SetInterpToNelia(false);

				switch (countAttack)
				{
				case 0:
					SetInterpToNelia(false);
					AnimInstance->Montage_Play(CombatMontage, 0.8f);
					AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
					break;
				case 1:
					SetInterpToNelia(false);
					//3
					AnimInstance->Montage_Play(CombatMontage, 0.8f);
					AnimInstance->Montage_JumpToSection(FName("Attack2_explosioin"), CombatMontage);
					break;
				case 2:
					SetInterpToNelia(false);

					AnimInstance->Montage_Play(CombatMontage, 0.8f);
					AnimInstance->Montage_JumpToSection(FName("Attack3"), CombatMontage);
					break;
				default:
					AnimInstance->Montage_Play(CombatMontage, 0.8f);
					AnimInstance->Montage_JumpToSection(FName("Default_Parrying"), CombatMontage);
					break;
				}

				EnemyAttackCounting++;
				if (EnemyAttackCounting > 2)
				{
					int RandAttack = FMath::FRandRange(0, 1);

					EnemyAttackCounting = 0;
					if (RandAttack == 0)
					{
						AnimInstance->Montage_Play(CombatMontage, 0.8f);
						AnimInstance->Montage_JumpToSection(FName("Attack4"), CombatMontage);
					}
					else if (RandAttack == 1)
					{
						AnimInstance->Montage_Play(CombatMontage, 0.8f);
						AnimInstance->Montage_JumpToSection(FName("Attack5"), CombatMontage);
					}
				}
				else
				{
					SetInterpToNelia(true);
				}
			}
		}
	}
}

