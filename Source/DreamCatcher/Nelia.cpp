//Fill out your copyright notice in the Description page of Project Settings.
#include "Nelia.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Enemy.h"
#include "MainAnimInstance.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "MainPlayerController.h"
#include "NeliaSaveGame.h"
#include "ItemStorage.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "DreamCatcherGameModeBase.h"
#include "GameFramework/GameMode.h"
#include "UserInterface.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Components/SphereComponent.h"

//#include "GameFramework/Pawn.h"      이거 쓰면 이상하게 상속받은 super클래스가 캐릭터에서 폰으로 바뀜

// Sets default values
ANelia::ANelia()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	GetCapsuleComponent()->SetCapsuleSize(20.f, 76.f);


	//FollowCamera를 카메라붐 끝 부분에 부착. FollowCamera는 이미 CameraBoom이 좌우로 돌기 때문에 같이 돌아갈 필요가 없어 false로 둠 단 CameraBoomd bUsePawnControlRotation을 false로두고
	//FollowCamera에서 true로 두면 Nelia를 기준으로 회전하지 않고 카메라를 기준으로 회전하기 때문에 캐릭터로부터 400.f 위치에서 홀로 돌아가게 됨
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(600.f);

	bOverlappingCombatSphere = false;
	bHasCombatTarget = false;
	targetIndex = 0;

	//카메라와 캐릭터가 동시 회전하는 걸 막는 역할 이렇게 설정하지 않으면 회전 시 캐릭터의 앞 모습을 절대 볼 수 없음.
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; //bOrientRotationToMovement는 자동으로 캐릭터의 이동방향에 맞춰, 회전 보간
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f); //키 입력 시 540씩 회전함
	GetCharacterMovement()->JumpZVelocity = 850.f; //점프하는 힘
	GetCharacterMovement()->AirControl = 0.2f; //중력의 힘
	
	MaxHealth = 100.f;
	Health = 100.f;
	MaxStamina = 150.f;
	Stamina = 120.f;

	Level = 1;
	Exp = 0.f;
	MaxExp = 100.f;

	Speed = 300.f;
	SprintingSpeed = 500.f;

	bShiftKeyDown = false;
	bPickup = false;
	bESCDown = false;

	//열거형 EMovementStatus와 EStaminaStatus를 ESS_Normal로 초기화 시켜줌
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	//StaminaDrainRate는 시간당 스테미나 소비량인 DeltaStamina 수식에 사용 DeltaStamina = StaminaDrainRate * DeltaTime;
	//MinSprintStamina는 아래 스위치 문에서 최소 스프린트 수치 이하일 경우 BelowMinimum(최소 이하) 상태로 변환하고 이 상태에서 스프린터키를 
	//누르게 되면 Exhausted 상태로 변화하여 달릴 수 없게됨
	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;	

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	bMovingForward = false;
	bMovingRight = false;

	bRoll = false;
	TraceDistance = 40.f;

	isClimb = false;
	onClimbLedge = false;
	isClimbUp = false;

	isBlock = false;
	
	checkPointCount = 0;
	spawnPointCheckNum = 0;

	wallLeftRight = 100.f;
	wallUpDown = 100.f;

	DeathDelay = 3.f;

	bTabKeyDown = false;
	bTargeting = false;

	bTakeDamage = false;
	
	bcanUseSkill = true;

	bCheckLevelUp = false;

	dialogueCheckNum = 0;

	PotionCount = 1;
}

void ANelia::BeginPlay()
{
	Super::BeginPlay();

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &ANelia::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &ANelia::CombatSphereOnOverlapEnd);

	MainPlayerController = Cast<AMainPlayerController>(GetController());


	Storage = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
}
 
//부모 클래스에 있는 점프에 대한 정보를 변경하기에는 문제가 발생할 수 있으므로 헤더에서 점프를 재정의해서 사용
void ANelia::Jump()
{
	FVector LaunchVelocity;
	bool bXYOverride = false;
	bool bZOverride = false;

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if ((MovementStatus != EMovementStatus::EMS_Death))
	{
		bJump = true;
		//만약 virtual void Jump() override; 상속 받은 함수에
		//ex) //#include "GameFramework/Pawn.h"      
		//이거 쓰면 이상하게 상속받은 super클래스가 캐릭터에서 폰으로 바뀜 이없음
		Super::Jump();

		if (isClimb)
		{
			//LaunchCharacter(LaunchVelocity, bXYOverride, bZOverride);
			LaunchCharacter(GetActorForwardVector() * -300.f, bXYOverride, bZOverride);
			isClimb = false;

			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
			GetCharacterMovement()->bOrientRotationToMovement = true;
			//ChangeModeToFly();
		}
	}
}

//GetCharacterMovement()의 벡터값을 0으로 초기화 및 EMovementMode를 Flying모드로 변환해준다.
void ANelia::ChangeModeToFly()
{
	if (EMovementMode::MOVE_Walking)
	{
		isClimb = true;
		GetCharacterMovement()->Velocity = FVector(0, 0, 0);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		
		//climb 도중 카메라 회전을 막기 위해 카메라 회전을 잠금
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}

//라인 트레이싱을 통해 플레이어 정면 벡터 * 40.f 그리고 정면 벡터 기준 위로 70.f만큼 위치에 감지되는 것이 있는지 확인
void ANelia::CanClimb()
{
	if (!isClimbUp)
	{
		FHitResult Hit;
		FVector Start = GetActorLocation();
		//TraceDistance는 시작할 때 40.f을 대입
		FVector End = Start + (GetActorForwardVector() * TraceDistance);
		FVector arriveEnd = (Start + (GetActorForwardVector())+FVector(0.f, 0.f, 70.f) + (GetActorForwardVector() * TraceDistance));

		FCollisionQueryParams TraceParams;
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);
		bool bArriveHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, arriveEnd, ECC_Visibility, TraceParams);
		
		
		/*DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);
		DrawDebugLine(GetWorld(), Start, arriveEnd, FColor::Red, false, 1, 0, 1);*/

		if (bHit)																																								
		{
			onClimbLedge = bArriveHit;

			ChangeModeToFly();
		}

		else
		{
			if (isClimb)
			{
				isClimb = false;
				GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
				GetCharacterMovement()->bOrientRotationToMovement = true;
			}
		}
		
		//정상 도착 시 애니메이션 수행
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (isClimb && !onClimbLedge)
		{
			isClimbUp = true;
			isClimb = false;
			AnimInstance->Montage_Play(ClimbTop_Two, 1.2f);

			//Timer(딜레이)
			FTimerHandle WaitHandle;

			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
				{
					isClimbUp = false;
					GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
					GetCharacterMovement()->bOrientRotationToMovement = true;
				}), 3.f, false);
		}
	}
}

//spacebar를 누르고 때면 호출
void ANelia::StopJumping()
{
	bJump = false;
}

void ANelia::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CanClimb();

	if (MovementStatus == EMovementStatus::EMS_Death) return;

	//시간 당 스테미나 소비량 = 스테미나소비율 * 프레임 
	float DeltaStamina = StaminaDrainRate * DeltaTime;

	//스태미나 상태(ESS)에 따른 움직임 상태(EMS) 예를 들어 스태미나가 충분할 시 EMS_Sprint로 500.f의 속도로 달리고 부족할 시 EMS_Normal 기본 달리기 300.f의 속도로 이동
	switch (StaminaStatus)
	{
	
	case EStaminaStatus::ESS_Normal:
		if (bShiftKeyDown && !isClimb)
		{
			if (Stamina - DeltaStamina <= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				Stamina -= DeltaStamina;
			}
			else
			{
				Stamina -= DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Sprinting);
			if (bMovingForward || bMovingRight)
			{
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
			else
			{
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
		}
		else if(!bShiftKeyDown) //shift key up
		{
			if (Stamina + DeltaStamina >= MaxStamina)
			{
				Stamina = MaxStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	case EStaminaStatus::ESS_BelowMinimum:
		if (bShiftKeyDown)
		{
			if (Stamina - DeltaStamina <= 0.f)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
				Stamina = 0;
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			else
			{
				Stamina -= DeltaStamina;
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
		}
		else //shift key up
		{
			if (Stamina + DeltaStamina >= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
				Stamina += DeltaStamina;
			}
			else 
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;

	case EStaminaStatus::ESS_Exhausted:
		if (bShiftKeyDown)
		{
			Stamina = 0.f;
		}
		else //shift key up
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;

	case EStaminaStatus::ESS_ExhaustedRecovering:
		if (Stamina + DeltaStamina >= MinSprintStamina)
		{
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
			Stamina += DeltaStamina;
		}
		else
		{
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;

	default:
		;

	}	

	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);
		SetActorRotation(InterpRotation);
	}
	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}
}

FRotator ANelia::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

void ANelia::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);					
	check(PlayerInputComponent);//입력받은 키를 확인

	PlayerInputComponent->BindAxis("MoveForward", this, &ANelia::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANelia::MoveRight);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ANelia::Jump);				
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ANelia::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ANelia::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ANelia::ShiftKeyUp);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &ANelia::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &ANelia::ESCUp);

	PlayerInputComponent->BindAction("SprintAttack", IE_Pressed, this, &ANelia::SprintAttack);

	PlayerInputComponent->BindAxis("Rolls", this, &ANelia::Rolls);

	PlayerInputComponent->BindAction("Pickup", IE_Pressed, this, &ANelia::PickupPress);

	PlayerInputComponent->BindAxis("turn", this, &ANelia::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ANelia::LookUp);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ANelia::Interact);

	PlayerInputComponent->BindAction("SkillOne", IE_Pressed, this, &ANelia::Skill);
	PlayerInputComponent->BindAction("SkillTwo", IE_Pressed, this, &ANelia::Skill);
	PlayerInputComponent->BindAction("SkillThree", IE_Pressed, this, &ANelia::Skill);
	PlayerInputComponent->BindAction("SkillFour", IE_Pressed, this, &ANelia::Skill);

	PlayerInputComponent->BindAction("Targeting", IE_Pressed, this, &ANelia::Targeting);
	PlayerInputComponent->BindAction("CancelTargeting", IE_Pressed, this, &ANelia::CancelTargeting);

	PlayerInputComponent->BindAction("Escape", IE_Pressed, this, &ANelia::Escape);

	PlayerInputComponent->BindAxis("CameraZoom", this, &ANelia::CameraZoom);


}

void ANelia::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			bOverlappingCombatSphere = true;
			for (int i = 0; i < Targets.Num(); i++)
			{
				if (Enemy == Targets[i]) 
				{
					return;
				}
			}
			Targets.Add(Enemy);
		}
	}
}

void ANelia::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);

		if (Enemy)
		{
			for (int i = 0; i < Targets.Num(); i++)
			{
				if (Enemy == Targets[i]) 
				{
					Targets.Remove(Enemy); //타겟팅 가능 몹 배열에서 제거

				}
			}
			if (Targets.Num() == 0)
			{
				bOverlappingCombatSphere = false;
			}

			if (CombatTarget == Enemy)
			{
				MainPlayerController->bTargetPointerVisible = false;
				MainPlayerController->RemoveTargetPointer();
				CombatTarget = nullptr;
				bHasCombatTarget = false;
			}
		}
	}
}

bool ANelia::CanMove(float Value)
{
	if (MainPlayerController)
	{
		if ((Value != 0.0f) &&
			(!bAttacking) &&
			(MovementStatus != EMovementStatus::EMS_Death)) {
			
			//일시정지 메뉴가 화면에 떴을 때는 이동하지 못하도록 하는 부분
			MainPlayerController->bPauseMenuVisible;
			
			return true;
		}
	}
	return false;
}

//마우스를 좌우로 움직일 경우 z값이 증가 감소함
void ANelia::Turn(float Value)
{
	if (CanMove(Value) && !isClimb)
	{
		AddControllerYawInput(Value);
	}
}

//마우스를 상하로 움직일 경우 y값이 증가 감소함
void ANelia::LookUp(float Value)
{
	if (CanMove(Value))
	{
		AddControllerPitchInput(Value);
	}
}

void ANelia::MoveForward(float Value)
{
	bMovingForward = false;

	if (!isClimb)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, Value);
		bMovingForward = true;
	}

	//속도를 높이려면 Velocity 값 변경
	if (isClimb)
	{
		wallUpDown = Value * 100.f;
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Z);

		if (Value != 0.0f)
		{
			GetCharacterMovement()->Velocity = FVector(0, 0, Value * 300.f);
		}
		AddMovementInput(Direction, wallUpDown);
	}
}

void ANelia::MoveRight(float Value)
{
	bMovingRight = false;

	if (!isClimb)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, Value);
		bMovingRight = true;
	}

	if (isClimb)											 
	{
		wallLeftRight = Value * 100.f;
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		if (Value != 0.0f)
		{
			GetCharacterMovement()->Velocity = FVector(Value * 150.f,0, 0);
		}

		AddMovementInput(Direction, wallLeftRight);
	}
}

void ANelia::PickupPress()
{
	bPickup = true;

	if (MovementStatus == EMovementStatus::EMS_Death) return;

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (ActiveOverlappingItem && !bAttacking && !EquippedWeapon)
	{
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);

		if (Weapon)
		{
			Weapon->Equip(this);
			
			SetActiveOverlappingItem(nullptr);
		}
	}
	if (ActiveOverlappingItem && !bAttacking && EquippedWeapon)
	{
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon)
		{
			Weapon->Equip(this);

			SetActiveOverlappingItem(nullptr);
			Attack();
		}
	}
	else
	{
		if (bAttacking)
		{
			saveAttack = true;
		}
		else
		{
			Attack();
		}
	}

	if (MainPlayerController->bDialogueVisible)
	{
		MainPlayerController->UserInterface->Interact();
	}
}

//ESC, Q 입력 시 해당 Pause 메뉴 창 팝업
void ANelia::ESCDown()
{
	bESCDown = true;

	if (MainPlayerController)
	{
		//메뉴창이 떠있으면 끄고 켜져있으면 끄는 함수
		MainPlayerController->TogglePauseMenu();
	}
}
void ANelia::ESCUp()
{
	bESCDown = false;
}

void ANelia::IncrementHealth(float Amount)
{
	if (Health + Amount >= MaxHealth)
	{
		Health = MaxHealth;
	}
	else
	{
		Health += Amount;
	}
}

void ANelia::DecrementHealth(float Amount)
{
	if (Health - Amount <= 0.f)
	{
		Health -= Amount;
		Die();
	}
	else
	{
		Health -= Amount;
	}
}

void ANelia::AddExp()
{
	RandomInt = FMath::RandRange(10, 30);
	Exp += RandomInt;

	if (Exp >= MaxExp)
	{
		bCheckLevelUp = true;
		Level+= 1;
		Exp = 0.f;
		
		this->EquippedWeapon->Damage += 10;
		//UE_LOG(LogTemp, Warning, TEXT("MaxExp%d"), MaxExp);
	}
}

void ANelia::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Death) return;

	MainPlayerController = Cast<AMainPlayerController>(GetController());
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.2f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
		bAttacking = false;
	}
	SetMovementStatus(EMovementStatus::EMS_Death);
	OnDeath();
}


//죽으면 애니메이션을 멈추고 스켈레톤 움직임도 정지
void ANelia::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

//위 Switch문에서 EMS_Sprinting 상태일 때는 속도를 SprintingSpeed=375.f로 값을 주고 그 외에는 Speed인 280.f
void ANelia::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = Speed;
	}
}

void ANelia::ShiftKeyDown()
{
	bShiftKeyDown = true;
}

void ANelia::ShiftKeyUp()
{
	bShiftKeyDown = false;
}

void ANelia::CameraZoom(const float Value)
{
	if (Value == 0.f || !Controller) return;

	const float NewTargetArmLength = CameraBoom->TargetArmLength + Value * ZoomSteps;
	CameraBoom->TargetArmLength = FMath::Clamp(NewTargetArmLength, MinZoomLength, MaxZoomLength);
}


void ANelia::Attack()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Death && EquippedWeapon && !MainPlayerController->bDialogueVisible && !isClimb)
	{
		bAttacking = true;
		SetInterpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		
		if (AnimInstance && CombatMontage)
		{
			int32 Section = AttackMotionCount;
			PlaySwingSound();

			//공격 키 입력을 연속적으로 빠르게 누를 경우 AttackMotionCount가 1씩 증가하며 3가지 공격 패턴을 수행
			//연속적이지 않을 경우 애니메이션 노티파이를 통해 값을 초기화 -> 첫 번째 공격 모션만 수행
			switch (Section)
			{
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 0.9f);
				AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
				AttackMotionCount++;
				break;
			case 1:
				AnimInstance->Montage_Play(CombatMontage, 0.9f);
				AnimInstance->Montage_JumpToSection(FName("Attack2"), CombatMontage);
				AttackMotionCount++;
				break;
			case 2:
				AnimInstance->Montage_Play(CombatMontage, 0.9f);
				AnimInstance->Montage_JumpToSection(FName("Attack3"), CombatMontage);
				AttackMotionCount++;

				break;
			default:
				break;
			}
		}
		
		if (AttackMotionCount > 2)
		{
			AttackMotionCount = 0;
		}
	}
}

void ANelia::Skill()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Death && EquippedWeapon && !MainPlayerController->bDialogueVisible && !isClimb)
	{
		bAttacking = true;
		SetInterpToEnemy(true);

		//MainPlayerController에 입력 받은 키 값을 pressSillNum에 대입
		pressSkillNum = MainPlayerController->CheckInputKey();
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && SkillMontage && bcanUseSkill && pressSkillNum <= Level)
		{	
			PlaySwingSound();
			switch (pressSkillNum)
			{
			case 1:
				AnimInstance->Montage_Play(SkillMontage, 0.9f);
				AnimInstance->Montage_JumpToSection(FName("Skill1"), SkillMontage);
				break;
			case 2:
				AnimInstance->Montage_Play(SkillMontage, 1.3f);
				AnimInstance->Montage_JumpToSection(FName("Skill2"), SkillMontage);
				break;
			case 3:
				AnimInstance->Montage_Play(SkillMontage, 1.f);
				AnimInstance->Montage_JumpToSection(FName("Skill3"), SkillMontage);
				break;
			default:
				AnimInstance->Montage_Play(SkillMontage, 2.f);
				AnimInstance->Montage_JumpToSection(FName("Heal"), SkillMontage);
				PotionCount -= 1;
				Health += 25;
				break;
			}
		}
		else if (AnimInstance && SkillMontage && pressSkillNum == 4 && PotionCount > 0)
		{
			AnimInstance->Montage_Play(SkillMontage, 2.f);
			AnimInstance->Montage_JumpToSection(FName("Heal"), SkillMontage);
			PotionCount -= 1;
			Health += 25;
		}
		else
		{
			SkillEnd();
		}
	}
}

void ANelia::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);
}

void ANelia::SkillEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);
}

void ANelia::ResetCombo()
{
	AttackMotionCount = 0;
	saveAttack = false;
	bAttacking = false;
}

void ANelia::SprintAttack()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Death && EquippedWeapon && !MainPlayerController->bDialogueVisible && !isClimb)
	{
		bAttacking = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && CombatMontage)
		{
			AnimInstance->Montage_Play(CombatMontage, 1.5f);
			AnimInstance->Montage_JumpToSection(FName("Attack4"), CombatMontage);
		}
	}
}

void ANelia::Targeting() 
{
	//적 검출 여부 확인
	if (bOverlappingCombatSphere) //There is a enemy in combatsphere
	{
		if (targetIndex >= Targets.Num()) //타겟인덱스가 총 타겟 가능 몹 수 이상이면 다시 0으로 초기화
		{
			targetIndex = 0;
		}
		if (MainPlayerController->bTargetPointerVisible)
		{
			bTargeting = false;
			MainPlayerController->RemoveTargetPointer();
		}
		bHasCombatTarget = true;
		bTargeting = true;
		CombatTarget = Targets[targetIndex];
		//현재 combat target의 이름을 전달하여 출력해줌
		//UE_LOG(LogTemp, Log, TEXT("%s"), *(CombatTarget->GetName()));
		targetIndex++;
		MainPlayerController->DisplayTargetPointer();
	}
}

void ANelia::CancelTargeting()
{	
	bTabKeyDown = true;
	if (ActiveOverlappingItem && !EquippedWeapon)
	{
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon)
		{
			Weapon->Equip(this);
			SetActiveOverlappingItem(nullptr);
		}
	}

	// 타깃 설정 해제
	if (CombatTarget)
	{
		bTargeting = false;
		if (MainPlayerController->bTargetPointerVisible)
		{
			MainPlayerController->RemoveTargetPointer();
		}
		CombatTarget = nullptr;
		bHasCombatTarget = false;
	}
}

void ANelia::SaveComboAttack()
{
	if (saveAttack)
	{
		saveAttack = false;
		AttackEnd();
		Attack();
	}
}

void ANelia::Rolls(float Value)
{
	if (!bRoll && MovementStatus != EMovementStatus::EMS_Death && (bMovingForward || bMovingRight) && !isClimb && GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftAlt))
	{
		bRoll = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (RollMontage && AnimInstance)
		{
			FName RollSectionName;

			switch (FMath::RoundToInt(Value))
			{
			case 1:
				RollSectionName = FName("RollFoward");
				break;
			case -1:
				RollSectionName = FName("RollBack");
				break;
			case 2:
				RollSectionName = FName("RollRight");
				break;
			case -2:
				RollSectionName = FName("RollLeft");
				break;
			default:
				break;
			}
			AnimInstance->Montage_Play(RollMontage, 1.2f);
			AnimInstance->Montage_JumpToSection(RollSectionName, RollMontage);
		}
	}
}

void ANelia::StopRoll()
{
	bRoll = false;
}

//장착한 무기에 소리 재생
void ANelia::PlaySwingSound()
{ 
	if (EquippedWeapon->SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}

void ANelia::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

float ANelia::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (Health - DamageAmount < 1.f)
	{
		Health = 0;
		bTakeDamage = false;
		Die();
		if (DamageCauser)
		{
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			if (Enemy)
			{
				Enemy->bHasValidTarget = false;
			}
		}
	}
	else
	{
		if (AnimInstance && !isBlock)
		{
			Health -= DamageAmount;
			bTakeDamage = true;
			int32 RandomHitMontion = FMath::RandRange(0, 2);
			FString HitNum = FString::Printf(TEXT("Hit%d"), RandomHitMontion);

			FName RandomHitNum(*HitNum);

			AnimInstance->Montage_Play(HitMontage, 1.3f);
			AnimInstance->Montage_JumpToSection(RandomHitNum, HitMontage);
		}
		else if (AnimInstance && isBlock)
		{
			bTakeDamage = false;
			AnimInstance->Montage_Play(CombatMontage, 1.3f);
			AnimInstance->Montage_JumpToSection(FName("Parry"), CombatMontage);
		}
	}

	return DamageAmount;
}
void ANelia::OnDeath()
{
	FTimerHandle DeathTimer;

	MainPlayerController = Cast<AMainPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	//MainPlayerController->UnPossess();

	APlayerCameraManager* playerCamera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (playerCamera)
	{
		playerCamera->StartCameraFade(0.f, 1.f, 3.f, FLinearColor::Black, false, true);
	}

	MainPlayerController->DiedHUD->SetVisibility(ESlateVisibility::Visible);

	GetWorld()->GetTimerManager().SetTimer(DeathTimer, FTimerDelegate::CreateLambda([&]()
		{
			Respawn();
			GetWorld()->GetTimerManager().ClearTimer(DeathTimer);
		}), DeathDelay, false);
}

void ANelia::Respawn()
{
	//FVector RespawnLocation = FVector(-5846.589844, 6323.025879, 7500.007812);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	//부활 시 컨트롤러 할당
	MainPlayerController->Possess(this);
	//블루프린트에서 위치 값을 지정함
	SetActorLocation(RespawnLocation);
	
	SetMovementStatus(EMovementStatus::EMS_Normal);

	APlayerCameraManager* playerCamera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	// 카메라 페이드 아웃
	if (playerCamera)
	{
		MainPlayerController->DiedHUD->SetVisibility(ESlateVisibility::Hidden);

		playerCamera->StartCameraFade(1.f, 0.f, 3.f, FLinearColor::Black, false, true);
	}

	if (AnimInstance && CombatMontage)
	{
		GetMesh()->bPauseAnims = false;
		GetMesh()->bNoSkeletonUpdate = false;

		AnimInstance->Montage_Play(CombatMontage, 0.7f);
		AnimInstance->Montage_JumpToSection(FName("Revive"), CombatMontage);
		Health += 50.f;
	}
}	

void ANelia::Escape()
{
	Respawn();
}


void ANelia::ReviveEnd()
{
	SetMovementStatus(EMovementStatus::EMS_Normal);
}

void ANelia::UpdateCombatTarget()
{	
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, EnemyFilter);

	if (OverlappingActors.Num() == 0)
	{
		if (MainPlayerController)
		{
			MainPlayerController->RemoveEnemyHealthBar();
		}
		return;
	}
	AEnemy* ClosestEnemy = Cast<AEnemy>(OverlappingActors[0]);
	if (ClosestEnemy)
	{
		FVector Location = GetActorLocation();
		float MinDistance = (ClosestEnemy->GetActorLocation() - Location).Size();

		for (auto Actor : OverlappingActors)
		{
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if (Enemy)
			{
				float DistanceToActor = (Enemy->GetActorLocation() - Location).Size();
				if (DistanceToActor < MinDistance)
				{
					MinDistance = DistanceToActor;
					ClosestEnemy = Enemy;
				}
			}
		}
		SetCombatTarget(ClosestEnemy);
		bHasCombatTarget = true;
	}
}

void ANelia::SaveGame()
{
	UNeliaSaveGame* SaveGameInstance = Cast<UNeliaSaveGame>(UGameplayStatics::CreateSaveGameObject(UNeliaSaveGame::StaticClass()));
	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Level = Level;
	SaveGameInstance->CharacterStats.Exp = Exp;
	SaveGameInstance->CharacterStats.MaxExp = MaxExp;
	SaveGameInstance->CharacterStats.PotionCount = PotionCount;
	LevelName = GetWorld()->GetMapName();
	LevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	SaveGameInstance->LevelName = LevelName;
	//UE_LOG(LogTemp, Warning, TEXT("%s"), *LevelName);

	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
}

void ANelia::LoadGame()
{
	UNeliaSaveGame* LoadGameInstance = Cast<UNeliaSaveGame>(UGameplayStatics::CreateSaveGameObject(UNeliaSaveGame::StaticClass()));

	LoadGameInstance = Cast<UNeliaSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	MainPlayerController = Cast<AMainPlayerController>(GetController());

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Level = LoadGameInstance->CharacterStats.Level;
	Exp = LoadGameInstance->CharacterStats.Exp;
	MaxExp = LoadGameInstance->CharacterStats.MaxExp;
	PotionCount = LoadGameInstance->CharacterStats.PotionCount;

	SetActorLocation(LoadGameInstance->CharacterStats.Location);
	SetActorRotation(LoadGameInstance->CharacterStats.Rotation);


	/*if (LoadGameInstance->CharacterStats.LevelName != TEXT(""))
	{
		LevelName = (*LoadGameInstance->CharacterStats.LevelName);

		SwitchLevel(LevelName);
	}*/
}

//현재 월드를 받아오고 현재 맵 이름을 CurrentLevel에 입력 CurrentLevelName과 매개변수 LevleName이 다르면 LevleName으로 이동
//void ANelia::SwitchLevel(FName LevelNameSwitch)
//{
//	UWorld* World = GetWorld();
//	if (World)
//	{
//		FString CurrentLevel = World->GetMapName();
//
//		FName CurrentLevelName(*CurrentLevel);
//		if (CurrentLevelName != LevelNameSwitch)
//		{
//			SaveGame();
//			UGameplayStatics::OpenLevel(World, CurrentLevelName);
//		}
//	}
//}

//e 키 입력 시 수행
void ANelia::Interact()
{	
	if (bCanUseDialogue)
	{
		if (MainPlayerController->UserInterface != nullptr && (MainPlayerController->UserInterface->CurrentState !=3))
		{
			//첫 대화 시작시에는 CurrentState가 0이므로 Interact의 if문을 모두 만족하지 못하고 나오게 되고 
			MainPlayerController->UserInterface->Interact();
		}

		//e 눌렀을 때 첫 대화인 경우 여기에 있는 if문을 수행하게 된다.
		if (!MainPlayerController->bDialogueVisible)
		{
			MainPlayerController->DisplayDialogue();
		}
	}
}
