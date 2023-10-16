// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Nelia.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "MainPlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "UserInterface.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UBlueprint> ExpItem(TEXT("Blueprint'/Game/Blueprints/Dream_BP.Dream_BP'"));
	static ConstructorHelpers::FObjectFinder<UBlueprint> ShootBP(TEXT("Blueprint'/Game/Enemy/Enemy1Stage/Boss/Attack_BP/ShootBase.ShootBase'"));

	if (ExpItem.Object)
	{
		ExpBlueprint = (UClass*)ExpItem.Object->GeneratedClass;
	}

	if (ShootBP.Object)
	{
		ShootBlueprint = (UClass*)ShootBP.Object->GeneratedClass;
	}
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
	bOverlappingAgroSphere = false;

	Health = 100.f;
	MaxHealth = 100.f;
	Damage = 10.f;

	AttackMinTime = 0.3f;
	AttackMaxTime = 0.8f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	DeathDelay = 2.f;

	bHasValidTarget = false;

	InterpSpeed = 25.f;
	bInterpToNelia = false;

	bSecuringKill = false;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	Nelia = Cast<ANelia>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	MainPlayerController = Cast<AMainPlayerController>(Nelia->GetController());

	//EnemyHUD = CreateWidget<UUserWidget>(this, HUDAsset);

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
	if (!AIController)
	{
		AIController = Cast<AAIController>(GetController());
	}
	
	if (bInterpToNelia && CombatTarget)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Interptonelia and Combattarget"));
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);


		SetActorRotation(InterpRotation);
		//UE_LOG(LogTemp, Warning, TEXT("TICK IN SETACTORLOCATION"));
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (AIController)
		{
			NeliaLocation = CombatTargetLocation;
		}
	}*/
}

FRotator AEnemy::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

void AEnemy::SetInterpToNelia(bool Interp)
{
	bInterpToNelia = Interp;
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
		ANelia* Target = Cast<ANelia>(OtherActor);

		//2023-6-24 if ���ǹ��� !bAttacking �߰��� - ĳ���Ͱ� ���� �߿� target������ �̵��ϴ� ���� �ذ��� �������� �� 
		if (Target)
		{
			MoveToTarget(Target);
			bOverlappingAgroSphere = true;
			MainPlayerController->DisplayEnemyHealthBar();
		}

	}
}

//���� ����� �ڱ� �ڽ��� �ƴ��� Ȯ���ϰ� ��ȿ Ÿ�� ������ �������� �ΰ� Nelia�� CombatTarget�� Enemy�� ��� ����� nullptr�� �ʱ�ȭ �����ش�.
//���� Enemy ���°� idle�� ��� AIController�� �����.
void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		ANelia* Target = Cast<ANelia>(OtherActor);

		if (Target)
		{
			bHasValidTarget = false;
			 
			if (Target->CombatTarget == this)
			{
				Target->SetCombatTarget(nullptr);
				bOverlappingAgroSphere = false;
				//Nelia->UpdateCombatTarget();
			}

			Target->SetHasCombatTarget(false);

			Target->UpdateCombatTarget();

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
		ANelia* Target = Cast<ANelia>(OtherActor);
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);

		//2023-6-24 if ���ǹ��� !bAttacking �߰��� - ĳ���Ͱ� ���� �߿� target������ �̵��ϴ� ���� �ذ��� �������� �� 
		if (Target && !bAttacking)
		{
			bHasValidTarget = true;

			Target->SetCombatTarget(this);
			Target->SetHasCombatTarget(true);
			
			Target->UpdateCombatTarget();

			CombatTarget = Target;
			bOverlappingCombatSphere = true;
			
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
		}
	}
}
void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherComp)
	{
		ANelia* Target = Cast<ANelia>(OtherActor);

		if (Target)
		{
			bOverlappingCombatSphere = false;
			MoveToTarget(Target);
			CombatTarget = nullptr;

			if (Target->CombatTarget == this)
			{
				Target->SetCombatTarget(nullptr);
				Target->bHasCombatTarget = false;
				Target->UpdateCombatTarget();
			}
			if (Target->MainPlayerController)
			{
				USkeletalMeshComponent* NeliaMesh = Cast<USkeletalMeshComponent>(OtherComp);
				if (NeliaMesh) Target->MainPlayerController->DisplayEnemyHealthBar();
			}

			GetWorldTimerManager().ClearTimer(AttackTimer);
		}
	}
}

//Nelia���� �̵��ϵ��� Ÿ������ ��� ���� AIController�� ������ ������: navpath�� ���� moveto�� �����ϱ� ������ �÷��̾ ���߿� �ִ� ��� ������ �Ұ���
void AEnemy::MoveToTarget(ANelia* Target)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if (AIController && !bAttacking)
	{
		FAIMoveRequest MoveRequest;
		//Ÿ���� ��ǥ ���ͷ� ����
		MoveRequest.SetGoalActor(Target);
		//�� �� ���� ���� �� Ÿ������ �ν�
		MoveRequest.SetAcceptanceRadius(40.0f);
		
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
		ANelia* Target = Cast<ANelia>(OtherActor);
		if (Target)
		{
			if (Target->HitParticles)
			{
				const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("TipSocket");
				if (TipSocket)
				{
					FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Target->HitParticles, SocketLocation, FRotator(0.f), false);
				}
			}
		
			if (Target->HitSound)
			{
				UGameplayStatics::PlaySound2D(this, Target->HitSound);
			}
			/////////
			if (DamageTypeClass)
			{
				UGameplayStatics::ApplyDamage(Target, Damage, AIController, this, DamageTypeClass);
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
//2023-06-27 �������� distanctocombat �� switch �Է� ���� DistanceToCombatInt�� ����
void AEnemy::Attack()
{
	/*DistanceToCombat = FVector::Distance(this->GetActorLocation(), Nelia->GetActorLocation());
	int DistanceToCombatInt = FMath::TruncToInt(DistanceToCombat % 600);*/
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
			if (AnimInstance && CombatMontage)
			{
				int32 Section = EnemyAttackCount;
				//2023-6-24 �������� �Ϸ��� �Ʒ� SECTIOIN�� �����ؾ���
				// int AttackNum = FMath::RandRange(1, 5);

				switch (Section)
				{
				case 0:
					AnimInstance->Montage_Play(CombatMontage, 1.2f);
					AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
					break;
				case 1:
					AnimInstance->Montage_Play(CombatMontage, 1.2f);
					AnimInstance->Montage_JumpToSection(FName("Attack2"), CombatMontage);
					break;
				default:
					if (world)
					{
						FActorSpawnParameters SpawnParams;
						SpawnParams.Owner = this;
						FRotator rotator;
						FVector SpawnLocation = GetActorLocation();

						world->SpawnActor<AActor>(ShootBlueprint, SpawnLocation, rotator, SpawnParams);
					}
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
void AEnemy::AttackEnd()
{
	bAttacking = false;
	SetInterpToNelia(false);
	if (bOverlappingCombatSphere)
	{
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);

		//2023-08-02 Ÿ�̸� �α� 
		//UE_LOG(LogTemp, Warning, TEXT("Timer, %f"), AttackTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
	}

	//!bAttacking ���� �߰� 2023-08-02 ����
	else if (bOverlappingAgroSphere && !bAttacking)
	{
		MoveToTarget(Nelia);
	}
}


//Enemy ü�� - ���� ���� ���������� 0���� ������ �� �ش� ��������ŭ ���ְ� ���� �����̹Ƿ� Nelia�� Ÿ���� ������Ʈ ���ش�.
float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	//2023-07-23 ��������
	//MainPlayerController = Cast<AMainPlayerController>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetController());

	if (GetWorld())
	{
		MainPlayerController = Cast<AMainPlayerController>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetController());
	}

	//2023-08-08 ���� targeting ǥ�ð� ���� ���� �������� ���� �ϰ� ���� 
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
			//UE_LOG(LogTemp, Warning, TEXT("IS OKAY"));			
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(CombatMontage, 1.f);
				AnimInstance->Montage_JumpToSection(FName("Hit"), CombatMontage);
			}
		}
	}

	return DamageAmount;
}

//Damage Causer�� Enemy���� ������ ���ϴ� �÷��̾ �ǹ�
//TakeDamage���� ü�� �̻��� �������� �޾� �װԵǸ� ������ ���� ����� Die �Լ��� �����Ͽ� �� ����� �´� ��� Ÿ������ �����Ѵ�.
void AEnemy::Die(AActor* Causer)
{
	FTimerHandle WaitHandle;
	MainPlayerController = Cast<AMainPlayerController>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetController());

	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);
	UWorld* world = GetWorld();
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}
	
	MainPlayerController->RemoveEnemyHealthBar();
	
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollisionLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bAttacking = false;
	
	ANelia* Target = Cast<ANelia>(Causer);
	if (Target)
	{
		Target->UpdateCombatTarget();
	}
}

//���� �Ŀ��� �ִϸ��̼��� ���߰� ���̷��浵 �����ش�. //////////// �ٵ� ���⼭ Ÿ�̸Ӵ� �� ���°���?
void AEnemy::DeathEnd()
{
	bSecuringKill = false;
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
	UWorld* world = GetWorld();

	Destroy();

	if (world)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		FRotator rotator;
		FVector SpawnLocation = GetActorLocation();

		int32 NumToSpawn = FMath::RandRange(1, 2);  // ������ ���� ���� (1���� 10 ������ ������ ���� ����)

		FVector OffsetRange(60.f, 40.f, 40.f);

		for (int32 i = 0; i < NumToSpawn; i++)
		{
			FVector RandomOffset = FVector(FMath::RandRange(-OffsetRange.X, OffsetRange.X), FMath::RandRange(-OffsetRange.Y, OffsetRange.Y), -30.f);
			FVector SpawnOffset = SpawnLocation + RandomOffset;

			world->SpawnActor<AActor>(ExpBlueprint, SpawnLocation, rotator, SpawnParams);
		}
	}
}