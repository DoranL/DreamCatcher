// Fill out your copyright notice in the Description page of Project Settings.


#include "FollowCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "FollwerAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Nelia.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "Enemy.h"	
#include "MainPlayerController.h"
#include "TimerManager.h"

//// Sets default values
AFollowCharacter::AFollowCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//AgroSphere�� Rampage�� Capsule Component�� �����Ǿ� �ִ� �ڽ� ������Ʈ�� ������ 600.f�� ������ ��Ÿ���� �� ���� �ȿ� Nelia�� ���� ��� ������ �����Ѵ�.
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroShere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(600.f);

	/////////////////////////////////////////////////////////
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);

	//CombatSphere�� AgroSphereó�� Capsule Component�� �����Ǿ� �ְ� �������� 75.f �� ���� �� Nelia�� ������ �������°� �ȴ�.
	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(75.f);

	bOverlappingCombatSphere = false;

	Health = 75.f;
	MaxHealth = 100.f;
	Damage = 10.f;

	AttackMinTime = 0.5f;
	AttackMaxTime = 3.5f;

	ArcherMovementStatus = EArcherMovementStatus::EMS_Idle;




	bHasValidTarget = false;

	InterpSpeed = 15.f;
	bInterpToNelia = false;
}
void AFollowCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Enemy ��Ʈ�ѷ��� �޾� �־��ְ� ���� ���� ���� ������ ��� �ش� �Լ��� ȣ�� ex) CombatCollisionLeft�� ������ ��� CombatOnOverlapBegin() ȣ��
	AIController = Cast<AAIController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AFollowCharacter::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AFollowCharacter::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AFollowCharacter::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AFollowCharacter::CombatSphereOnOverlapEnd);
}
void AFollowCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bInterpToNelia && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);


		SetActorRotation(InterpRotation);
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (AIController)
		{
			NeliaLocation = CombatTargetLocation;
		}
	}
}

FRotator AFollowCharacter::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

void AFollowCharacter::SetInterpToNelia(bool Interp)
{
	bInterpToNelia = Interp;
}

void AFollowCharacter::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			MoveToTarget(Nelia);
		}
	}
}

//���� ����� �ڱ� �ڽ��� �ƴ��� Ȯ���ϰ� ��ȿ Ÿ�� ������ �������� �ΰ� Nelia�� CombatTarget�� Enemy�� ��� ����� nullptr�� �ʱ�ȭ �����ش�.
//���� Enemy ���°� idle�� ��� AIController�� �����.
void AFollowCharacter::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			bHasValidTarget = false;

			Nelia->SetHasCombatTarget(false);

			Nelia->UpdateCombatTarget();

			//SetArcherMovementStatus(EArcherMovementStatus::EMS_Idle);
			if (AIController)
			{
				AIController->StopMovement();
			}
		}
	}
}

//�� if�� ����� ���� ���� Nelia�̸� ��ȿ Ÿ���� true�� �ΰ�
void AFollowCharacter::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			bHasValidTarget = true;

			Nelia->SetHasCombatTarget(true);

			Nelia->UpdateCombatTarget();

			CombatTarget = Nelia;
			bOverlappingCombatSphere = true;
			Attack();
		}
	}
}
void AFollowCharacter::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherComp)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			bOverlappingCombatSphere = false;
			MoveToTarget(Nelia);
			CombatTarget = nullptr;

			if (Nelia->MainPlayerController)
			{
				USkeletalMeshComponent* NeliaMesh = Cast<USkeletalMeshComponent>(OtherComp);
				if (NeliaMesh) Nelia->MainPlayerController->RemoveEnemyHealthBar();
			}

			GetWorldTimerManager().ClearTimer(AttackTimer);
		}
	}
}

//Nelia���� �̵��ϵ��� Ÿ������ ��� ���� AIController�� ������ ///////////////////////////////////////////////////////////////////////////////
void AFollowCharacter::MoveToTarget(ANelia* Target)
{
	SetEnemyMovementStatus(EArcherMovementStatus::EMS_MoveToTarget);

	if (AIController)
	{
		FAIMoveRequest MoveRequest;
		//Ÿ���� ��ǥ ���ͷ� ����
		MoveRequest.SetGoalActor(Target);
		//�� �� ���� ���� �� Ÿ������ �ν�
		MoveRequest.SetAcceptanceRadius(10.0f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);

	}
}

void AFollowCharacter::Attack()
{
	if (bHasValidTarget)
	{
		if (AIController)
		{
			AIController->StopMovement();
			SetEnemyMovementStatus(EArcherMovementStatus::EMS_Attacking);
		}

		if (!bAttacking)
		{
			bAttacking = true;
			SetInterpToNelia(true);

			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				int32 Section = EnemyAttackCount;
				switch (Section)
				{
				case 0:
					AnimInstance->Montage_Play(AttackMontage, 1.f);
					AnimInstance->Montage_JumpToSection(FName("Attack"), AttackMontage);
					break;
				case 1:
					AnimInstance->Montage_Play(AttackMontage, 1.f);
					AnimInstance->Montage_JumpToSection(FName("Attack2"), AttackMontage);
					break;
				default:
					break;
				}
				EnemyAttackCount++;
				if (EnemyAttackCount > 1)
				{
					EnemyAttackCount = 0;
				}
			}
		}
	}
}

//���� ��� ������ �������� �������ش� �κ� play �� ���� �ִϸ��̼� ������ ������ ������ �ٸ� ���� �� �� �ִ�.
void AFollowCharacter::AttackEnd()
{
	bAttacking = false;
	SetInterpToNelia(false);
	if (bOverlappingCombatSphere)
	{
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AFollowCharacter::Attack, AttackTime);
	}
}

void AFollowCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
