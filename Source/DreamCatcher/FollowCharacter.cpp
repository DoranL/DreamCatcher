// Fill out your copyright notice in the Description page of Project Settings.


#include "FollowCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "FollowerAnimInstance.h"
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


AFollowCharacter::AFollowCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UBlueprint> ProjectileBlueprint(TEXT("Blueprint'/Game/Blueprints/Projectile_BP.Projectile_BP'"));

	if (ProjectileBlueprint.Object)
	{
		Projectile = (UClass*)ProjectileBlueprint.Object->GeneratedClass;
	}

	//AgroSphere은 Rampage의 Capsule Component에 부착되어 있는 자식 컴포넌트로 반지름 600.f는 범위를 나타내며 이 범위 안에 Nelia가 있을 경우 추적을 시작한다.
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroShere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(600.f);

	/////////////////////////////////////////////////////////
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);

	//CombatSphere도 AgroSphere처럼 Capsule Component에 부착되어 있고 반지름은 75.f 이 범위 내 Nelia가 있으면 전투상태가 된다.
	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(75.f);

	///////////////////////각각 왼팔 오른팔에 부착해둔 CombatCollision EnemySocket을 부착해줌 그 위치에 박스 컴포넌트가 생성된다.
	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("Arrow_Socket"));

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

	//Enemy 컨트롤러를 받아 넣어주고 각각 범위 내에 감지될 경우 해당 함수를 호출 ex) CombatCollisionLeft에 감지될 경우 CombatOnOverlapBegin() 호출
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
			UE_LOG(LogTemp, Warning, TEXT("ing ing check"));
		}
	}
}

//검출 대상이 자기 자신이 아닌지 확인하고 유효 타겟 변수를 없음으로 두고 Nelia의 CombatTarget이 Enemy일 경우 대상을 nullptr로 초기화 시켜준다.
//만약 Enemy 상태가 idle일 경우 AIController를 멈춘다.
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

//위 if문 내용과 동일 만약 Nelia이면 유효 타겟을 true로 두고
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
			SpawnProjectile();

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

//콜리전을 활성화시켜 주는 함수 CombatCollsion, CombatCollisionL은 실제로 내가 설정해준 Enemy의 왼손과 오른손 콜리전 
//만약 콜리전이 활성화된 상태에서 휘두르는 소리가 있을 경우 소리를 재생한다.
void AFollowCharacter::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	//if (SwingSound)
	//{
	//	UGameplayStatics::PlaySound2D(this, SwingSound);
	//}
}

//위 내용과 반대의 내용 왼손과 오른손에 있는 콜리전을 NoCollsion을 통해 비활성화 시켜준다.
void AFollowCharacter::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

//Nelia에게 이동하도록 타겟으로 잡고 만약 AIController가 있으면 ///////////////////////////////////////////////////////////////////////////////
void AFollowCharacter::MoveToTarget(ANelia* Target)
{
	SetEnemyMovementStatus(EArcherMovementStatus::EMS_MoveToTarget);

	if (AIController)
	{
		FAIMoveRequest MoveRequest;
		//타겟을 목표 액터로 지정
		MoveRequest.SetGoalActor(Target);
		//이 원 내에 있을 때 타겟으로 인식
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

//공격 모션 간격을 랜덤으로 설정해준는 부분 play 시 공격 애니메이션 간격이 실행할 때마다 다른 것을 알 수 있다.
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

void AFollowCharacter::SpawnProjectile()
{
	if (Projectile)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		FRotator SpawnRotation = GetActorRotation();
		FVector SpawnLocation = GetActorLocation();

		GetWorld()->SpawnActor<AActor>(Projectile, SpawnLocation, SpawnRotation, SpawnParams);
	}
}