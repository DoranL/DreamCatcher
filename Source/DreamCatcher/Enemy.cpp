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
#include "MainPlayerController.h"

// Sets default values
AEnemy::AEnemy()
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

	///////////////////////���� ���� �����ȿ� �����ص� CombatCollision EnemySocket�� �������� �� ��ġ�� �ڽ� ������Ʈ�� �����ȴ�.
	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("EnemySocket"));

	CombatCollisionLeft = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollisionL"));
	CombatCollisionLeft->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("EnemySocket1"));

	bOverlappingCombatSphere = false;

	Health = 75.f;
	MaxHealth = 100.f;
	Damage = 10.f;

	AttackMinTime = 0.5f;
	AttackMaxTime = 3.5f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	DeathDelay = 3.f;

	bHasValidTarget = false;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	//Enemy ��Ʈ�ѷ��� �޾� �־��ְ� ���� ���� ���� ������ ��� �ش� �Լ��� ȣ�� ex) CombatCollisionLeft�� ������ ��� CombatOnOverlapBegin() ȣ��
	AIController = Cast<AAIController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);

	CombatCollisionLeft->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	CombatCollisionLeft->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);

	/// <summary>
	/// ////////////////////////////////////////////////////////////////////
	/// </summary>
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	CombatCollisionLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollisionLeft->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollisionLeft->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollisionLeft->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	//���Ͱ� �÷��̾ ���� �� ī�޶� �������� �� ���� �ڵ�(Mesh(�� �׵θ�), capsule(���� ������ ĸ��)�� ī�޶� �þ߿� �ְԵǸ�
	//�Ʒ� �ڵ尡 ���� �� ĳ���͸� �����ֱ� ���� ������ ������� �Ǵµ� �� �ɸ� �� ó�� �������� �� ���� �����ϵ��� �ڵ� �ۼ�
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

//���� �� ����� ����� �ڱ� �ڽ��� �ƴϰ� �ڱ� �ڽ��� ���� ���� ������ ��� OtherActor�� Nelia���� Ȯ���ϰ� ���� �� ��� Ÿ������ �̵�
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

//���� ����� �ڱ� �ڽ��� �ƴ��� Ȯ���ϰ� ��ȿ Ÿ�� ������ �������� �ΰ� Nelia�� CombatTarget�� Enemy�� ��� ����� nullptr�� �ʱ�ȭ �����ش�.
//���� Enemy ���°� idle�� ��� AIController�� �����.
void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			bHasValidTarget = false;
			 
			if (Nelia->CombatTarget == this)
			{
				Nelia->SetCombatTarget(nullptr);
				//Nelia->UpdateCombatTarget();
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
}

//�� if�� ����� ���� ���� Nelia�̸� ��ȿ Ÿ���� true�� �ΰ�
void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive())
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			bHasValidTarget = true;

			Nelia->SetCombatTarget(this);
			Nelia->SetHasCombatTarget(true);
			
			Nelia->UpdateCombatTarget();

			CombatTarget = Nelia;
			bOverlappingCombatSphere = true;
			Attack();
		}
	}
}
void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherComp)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
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
}

//Nelia���� �̵��ϵ��� Ÿ������ ��� ���� AIController�� ������ ///////////////////////////////////////////////////////////////////////////////
void AEnemy::MoveToTarget(ANelia* Target)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

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

//�ش� OtherActor�� �ڽ��� �ƴ϶� Nelia�̰� Nelia�� ���� �� �ٿ��� HitParticle�� ���� ��� Enemy�� �����ص� TipSocket�� �ҷ����� 
// TipSocket�� ������ ������ �ִ� �ش� ��ġ���� HitParticle�� ��Ÿ�� 
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
			/////////
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


//�ݸ����� Ȱ��ȭ���� �ִ� �Լ� CombatCollsion, CombatCollisionL�� ������ ���� �������� Enemy�� �޼հ� ������ �ݸ��� 
//���� �ݸ����� Ȱ��ȭ�� ���¿��� �ֵθ��� �Ҹ��� ���� ��� �Ҹ��� ����Ѵ�.
void AEnemy::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CombatCollisionLeft->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

//�� ����� �ݴ��� ���� �޼հ� �����տ� �ִ� �ݸ����� NoCollsion�� ���� ��Ȱ��ȭ �����ش�.
void AEnemy::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollisionLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

//Enemy�� ���� �ʾҰ� ��ȿ Ÿ���� ���� ��� ���� AIController�� ���� ��� �̵��� ���߰� ���¸� ���� ���·� �����Ѵ�. (�̵��� �ϸ鼭 �����ϴ� ���� �����ϱ� ����
//���� ���� ���� ���°� �ƴ϶�� ���� ���¸� Ȯ���ϴ� bool ������ true�� �ΰ� Enemy�� AnimInstance�� �������� Switch ���� ���� ���� ����� Attack, Attack1, JumpAttack ������� �����ϵ��� ����
//�� CombatMontage�� �����ص� 3���� ���� ����� ���� ���� �� JumpToSection�� ���� �̸��� ���� �ش� �������� �����Ǿ� ����ȴ�.
void AEnemy::Attack()
{
	if (Alive() && bHasValidTarget)
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
				int32 Section = EnemyAttackCount;
				switch (Section)
				{
				case 0:
					AnimInstance->Montage_Play(CombatMontage, 1.3f);
					AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
					break;
				case 1:
					AnimInstance->Montage_Play(CombatMontage, 1.f);
					AnimInstance->Montage_JumpToSection(FName("Attack1"), CombatMontage);
					break;
				case 2:
					AnimInstance->Montage_Play(CombatMontage, 1.7f);
					AnimInstance->Montage_JumpToSection(FName("JumpAttack"), CombatMontage);
					break;
				default:
					;
				}
				EnemyAttackCount++;
				if (EnemyAttackCount > 2)
				{
					EnemyAttackCount = 0;
				}
			}
		}
	}
}

//���� ��� ������ �������� �������ش� �κ� play �� ���� �ִϸ��̼� ������ ������ ������ �ٸ� ���� �� �� �ִ�.
void AEnemy::AttackEnd()
{
	bAttacking = false;
	if (bOverlappingCombatSphere)
	{
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
	}
}


//Enemy ü�� - ���� ���� ���������� 0���� ������ �� �ش� ��������ŭ ���ְ� ���� �����̹Ƿ� Nelia�� Ÿ���� ������Ʈ ���ش�.
float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f)
	{
		Health -= DamageAmount;
		Die(DamageCauser);
	}
	else
	{
		Health -= DamageAmount;
	}

	return DamageAmount;
}

//���¸� ���� ���·� �ΰ� DeathMontage���� �ִϸ��̼��� ���� �տ��� �����ص� Collision�� Capsule Component�� ��� �����ش�.
//���� Nelia�� ���� ����� ������Ʈ�� �Ͽ� ���ο� ����� ������ �� �ֵ��� ����.
void AEnemy::Die(AActor* Causer)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.35f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollisionLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bAttacking = false;

	ANelia* Nelia = Cast<ANelia>(Causer);
	if (Nelia)
	{
		Nelia->UpdateCombatTarget();
	}
}

//���� �Ŀ��� �ִϸ��̼��� ���߰� ���̷��浵 �����ش�. //////////// �ٵ� ���⼭ Ÿ�̸Ӵ� �� ���°���?
void AEnemy::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

    GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay);
}

bool AEnemy::Alive()
{
	return GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead;
}

void AEnemy::Disappear()
{
	Destroy();
}