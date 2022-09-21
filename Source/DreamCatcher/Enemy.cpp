// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Nelia.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Nelia.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroShere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(600.f);
	//
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(75.f);

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("EnemySocket"));

	bOverlappingCombatSphere = false;

	Health = 75.f;
	MaxHealth = 100.f;
	Damage = 10.f;

	AttackMinTime = 0.5f;
	AttackMaxTime = 3.5f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	//몬스터가 플레이어를 때릴 때 카메라에 가려지는 것 막는 코드(Mesh(적 테두리), capsule(내가 설정한 캡슐)이 카메라 시야에 있게되면
	//아래 코드가 없을 시 캐릭터를 보여주기 위해 앞으로 당겨지게 되는데 렉 걸린 것 처럼 보여지게 됨 따라서 무시하도록 코드 작성
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive())
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			MoveToTarget(Nelia);
		}
	}
}
void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
			if (AIController)
			{
				AIController->StopMovement();
			}
		}
	}
	/*if (OtherActor)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		{
			if (Nelia)
			{
				bHasValidTarget = false;
				if (Nelia->CombatTarget == this)
				{
					Nelia->SetCombatTarget(nullptr);
				}
				Nelia->SetHasCombatTarget(false);

				Nelia->UpdateCombatTarget();

				SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
				if (AIController)
				{
					AIController->StopMovement();
				}
			}
		}
	}*/
}
void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive())
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			Nelia->SetCombatTarget(this);

			CombatTarget = Nelia;
			bOverlappingCombatSphere = true;
			Attack();
		}
	}
	/*if (OtherActor && Alive())
	{
		ANelia* Nelia = Cast<AMain>(OtherActor);
		{
			if (Nelia)
			{
				bHasValidTarget = true;
				Nelia->SetCombatTarget(this);
				Nelia->SetHasCombatTarget(true);

				Nelia->UpdateCombatTarget();

				if (Nelia->MainPlayerController)
				{
					Nelia->MainPlayerController->DisplayEnemyHealthBar();
				}
				CombatTarget = Nelia;
				bOverlappingCombatSphere = true;

				float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
				GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
			}
		}
	}*/
}
void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			if (Nelia->CombatTarget == this)
			{
				Nelia->SetCombatTarget(nullptr);
				//Nelia->bHasCombatTarget = false;
				//Nelia->UpdateCombatTarget();
			}
			Nelia->SetCombatTarget(nullptr);
			bOverlappingCombatSphere = false;
			if (EnemyMovementStatus != EEnemyMovementStatus::EMS_Attacking)
			{
				MoveToTarget(Nelia);
				CombatTarget = nullptr;
			}
			GetWorldTimerManager().ClearTimer(AttackTimer);
		}
	}
	/*if (OtherActor && OtherComp)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		{
			if (Nelia)
			{
				bOverlappingCombatSphere = false;
				MoveToTarget(Nelia);
				CombatTarget = nullptr;

				if (Nelia->CombatTarget == this)
				{
					Nelia->SetCombatTarget(nullptr);
					Nelia->bHasCombatTarget = false;
					Nelia->UpdateCombatTarget();
				}

				if (Nelia->MainPlayerController)
				{
					USkeletalMeshComponent* NeliaMesh = Cast<USkeletalMeshComponent>(OtherComp);
					if (NeliaMesh) Nelia->MainPlayerController->RemoveEnemyHealthBar();
				}

				GetWorldTimerManager().ClearTimer(AttackTimer);
			}
		}
	}*/
}

//MoveToTarget이 플레이어 콜라이더랑 enemy 콜라이더랑 겹치게 되면 적이 플레이어 쪽으로 이동하도록 되어 있는 함수인듯?
void AEnemy::MoveToTarget(ANelia* Target)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if (AIController)
	{
		FAIMoveRequest MoveRequest;
		//타겟을 목표 액터로 지정
		MoveRequest.SetGoalActor(Target);
		//이 원 내에 있을 때 타겟으로 인식
		MoveRequest.SetAcceptanceRadius(10.0f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);

		/*auto PathPoints = NavPath->GetPathPoints();
		for (auto Point : PathPoints)
		{
			FVector Location = Point.Location;

			UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 8, FLinearColor::Red, 10.f, 1.5f);
		}*/
	}
}

void AEnemy::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{

		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			if (Nelia->HitParticles)
			{
				const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("TipSocket");
				if (TipSocket)
				{

					FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Nelia->HitParticles, SocketLocation, FRotator(0.f), false);
				}
			}
		
			if (Nelia->HitSound)
			{
				UGameplayStatics::PlaySound2D(this, Nelia->HitSound);
			}
			if (DamageTypeClass)
			{
				UGameplayStatics::ApplyDamage(Nelia, Damage, AIController, this, DamageTypeClass);
			}
		}
	}
}

void AEnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AEnemy::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

void AEnemy::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::Attack()
{
	if (Alive())
	{
		if (AIController)
		{
			AIController->StopMovement();
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
		}

		if (!bAttacking)
		{
			bAttacking = true;
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(CombatMontage, 0.5f);
				AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
			}
		}
	}
}

void AEnemy::AttackEnd()
{
	bAttacking = false;
	if (bOverlappingCombatSphere)
	{
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f)
	{
		Health -= DamageAmount;
		Die();//DamageCauser);
	}
	else
	{
		Health -= DamageAmount;
	}

	return DamageAmount;
}

void AEnemy::Die()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.35f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

//	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay);
}

bool AEnemy::Alive()
{
	return GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead;
}