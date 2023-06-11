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

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UBlueprint> ExpItem(TEXT("Blueprint'/Game/Blueprints/Dream_BP.Dream_BP'"));

	if (ExpItem.Object)
	{
		ExpBlueprint = (UClass*)ExpItem.Object->GeneratedClass;
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
	CombatCollision->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("EnemySocket"));

	CombatCollisionLeft = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollisionL"));
	CombatCollisionLeft->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("EnemySocket1"));

	bOverlappingCombatSphere = false;
	bOverlappingAgroSphere = false;

	Health = 100.f;
	MaxHealth = 100.f;
	Damage = 10.f;

	AttackMinTime = 0.5f;
	AttackMaxTime = 1.f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	DeathDelay = 2.f;

	bHasValidTarget = false;

	InterpSpeed = 15.f;
	bInterpToNelia = false;

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	//Enemy 컨트롤러를 받아 넣어주고 각각 범위 내에 감지될 경우 해당 함수를 호출 ex) CombatCollisionLeft에 감지될 경우 CombatOnOverlapBegin() 호출
	AIController = Cast<AAIController>(GetController());
	Nelia = Cast<ANelia>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	
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

	//몬스터가 플레이어를 때릴 때 카메라에 가려지는 것 막는 코드(Mesh(적 테두리), capsule(내가 설정한 캡슐)이 카메라 시야에 있게되면
	//아래 코드가 없을 시 캐릭터를 보여주기 위해 앞으로 당겨지게 되는데 렉 걸린 것 처럼 보여지게 됨 따라서 무시하도록 코드 작성
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
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

//범위 내 검출된 대상이 자기 자신이 아니고 자기 자신이 죽지 않은 상태일 경우 OtherActor가 Nelia인지 확인하고 맞을 시 대상 타겟으로 이동
void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive())
	{
		ANelia* Target = Cast<ANelia>(OtherActor);

		if (Target)
		{
			MoveToTarget(Target);
			bOverlappingAgroSphere = true;
		}
	}
}

//검출 대상이 자기 자신이 아닌지 확인하고 유효 타겟 변수를 없음으로 두고 Nelia의 CombatTarget이 Enemy일 경우 대상을 nullptr로 초기화 시켜준다.
//만약 Enemy 상태가 idle일 경우 AIController를 멈춘다.
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

//위 if문 내용과 동일 만약 Nelia이면 유효 타겟을 true로 두고
void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive())
	{
		ANelia* Target = Cast<ANelia>(OtherActor);
		
		if (Target)
		{
			bHasValidTarget = true;

			Target->SetCombatTarget(this);
			Target->SetHasCombatTarget(true);
			
			Target->UpdateCombatTarget();

			CombatTarget = Target;
			bOverlappingCombatSphere = true;
			Attack();
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
				if (NeliaMesh) Target->MainPlayerController->RemoveEnemyHealthBar();
			}

			GetWorldTimerManager().ClearTimer(AttackTimer);
		}
	}
}

//Nelia에게 이동하도록 타겟으로 잡고 만약 AIController가 있으면 ///////////////////////////////////////////////////////////////////////////////
void AEnemy::MoveToTarget(ANelia* Target)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if (AIController && !bAttacking)
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


//해당 OtherActor가 자신이 아니라 Nelia이고 Nelia를 공격 시 다오는 HitParticle이 있을 경우 Enemy에 부착해둔 TipSocket을 불러오고 
// TipSocket이 있으면 소켓이 있는 해당 위치에서 HitParticle을 나타냄 
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


//콜리전을 활성화시켜 주는 함수 CombatCollsion, CombatCollisionL은 실제로 내가 설정해준 Enemy의 왼손과 오른손 콜리전 
//만약 콜리전이 활성화된 상태에서 휘두르는 소리가 있을 경우 소리를 재생한다.
void AEnemy::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CombatCollisionLeft->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

//위 내용과 반대의 내용 왼손과 오른손에 있는 콜리전을 NoCollsion을 통해 비활성화 시켜준다.
void AEnemy::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollisionLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

//Enemy가 죽지 않았고 유효 타겟이 있을 경우 만약 AIController가 있을 경우 이동을 멈추고 상태를 공격 상태로 지정한다. (이동을 하면서 공격하는 것을 방지하기 위함
//만약 공격 중인 상태가 아니라면 공격 상태를 확인하는 bool 변수를 true로 두고 Enemy의 AnimInstance를 가져오고 Switch 문을 통해 공격 모션을 Attack, Attack1, JumpAttack 순서대로 실행하도록 설정
//한 CombatMontage에 설정해둔 3가지 공격 모션이 조건 만족 시 JumpToSection을 통해 이름이 같은 해당 섹션으로 점프되어 실행된다.
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
			SetInterpToNelia(true);

			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				int32 Section = EnemyAttackCount;
				switch (Section)
				{
				case 0:
					AnimInstance->Montage_Play(CombatMontage, 1.f);
					AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
					break;
				case 1:
					AnimInstance->Montage_Play(CombatMontage, 1.f);
					AnimInstance->Montage_JumpToSection(FName("Attack2"), CombatMontage);
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
void AEnemy::AttackEnd()
{
	bAttacking = false;
	SetInterpToNelia(false);
	if (bOverlappingCombatSphere)
	{
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
	}

	else if (bOverlappingAgroSphere)
	{
		
			MoveToTarget(Nelia);
		
	}
}


//Enemy 체력 - 공격 받은 데미지량이 0보다 작으면 그 해당 데미지만큼 빼주고 죽은 상태이므로 Nelia의 타겟을 업데이트 해준다.
float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (GetWorld())
	{
		MainPlayerController = Cast<AMainPlayerController>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetController());
	}

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
		//UE_LOG(LogTemp, Log, TEXT("DisplayEnemyHealthBarPlease"));

		Health -= DamageAmount;
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(CombatMontage, 1.f);
			AnimInstance->Montage_JumpToSection(FName("Hit"), CombatMontage);
		}
	}

	return DamageAmount;
}

//상태를 죽은 상태로 두고 DeathMontage에서 애니메이션을 실행 앞에서 설정해둔 Collision과 Capsule Component를 모두 지워준다.
//이후 Nelia의 공격 대상을 업데이트를 하여 새로운 대상을 공격할 수 있도록 해줌.
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

//죽은 후에는 애니메이션을 멈추고 스켈레톤도 없애준다. //////////// 근데 여기서 타이머는 왜 쓰는거지?
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
	UWorld* world = GetWorld();

	Destroy();

	if (world)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		FRotator rotator;
		FVector SpawnLocation = GetActorLocation();

		int32 NumToSpawn = FMath::RandRange(1, 2);  // 랜덤한 개수 설정 (1부터 10 사이의 값으로 변경 가능)

		FVector OffsetRange(60.f, 40.f, 40.f);

		for (int32 i = 0; i < NumToSpawn; i++)
		{
			FVector RandomOffset = FVector(FMath::RandRange(-OffsetRange.X, OffsetRange.X), FMath::RandRange(-OffsetRange.Y, OffsetRange.Y), -30.f);
			FVector SpawnOffset = SpawnLocation + RandomOffset;

			world->SpawnActor<AActor>(ExpBlueprint, SpawnLocation, rotator, SpawnParams);
		}
	}
}