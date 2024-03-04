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

//#include "GameFramework/Pawn.h"      �̰� ���� �̻��ϰ� ��ӹ��� superŬ������ ĳ���Ϳ��� ������ �ٲ�

// Sets default values
ANelia::ANelia()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	GetCapsuleComponent()->SetCapsuleSize(20.f, 76.f);


	//FollowCamera�� ī�޶�� �� �κп� ����. FollowCamera�� �̹� CameraBoom�� �¿�� ���� ������ ���� ���ư� �ʿ䰡 ���� false�� �� �� CameraBoomd bUsePawnControlRotation�� false�εΰ�
	//FollowCamera���� true�� �θ� Nelia�� �������� ȸ������ �ʰ� ī�޶� �������� ȸ���ϱ� ������ ĳ���ͷκ��� 400.f ��ġ���� Ȧ�� ���ư��� ��
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(600.f);

	bOverlappingCombatSphere = false;
	bHasCombatTarget = false;
	targetIndex = 0;

	//ī�޶�� ĳ���Ͱ� ���� ȸ���ϴ� �� ���� ���� �̷��� �������� ������ ȸ�� �� ĳ������ �� ����� ���� �� �� ����.
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; //bOrientRotationToMovement�� �ڵ����� ĳ������ �̵����⿡ ����, ȸ�� ����
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f); //Ű �Է� �� 540�� ȸ����
	GetCharacterMovement()->JumpZVelocity = 850.f; //�����ϴ� ��
	GetCharacterMovement()->AirControl = 0.2f; //�߷��� ��
	
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

	//������ EMovementStatus�� EStaminaStatus�� ESS_Normal�� �ʱ�ȭ ������
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	//StaminaDrainRate�� �ð��� ���׹̳� �Һ��� DeltaStamina ���Ŀ� ��� DeltaStamina = StaminaDrainRate * DeltaTime;
	//MinSprintStamina�� �Ʒ� ����ġ ������ �ּ� ������Ʈ ��ġ ������ ��� BelowMinimum(�ּ� ����) ���·� ��ȯ�ϰ� �� ���¿��� ��������Ű�� 
	//������ �Ǹ� Exhausted ���·� ��ȭ�Ͽ� �޸� �� ���Ե�
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
 
//�θ� Ŭ������ �ִ� ������ ���� ������ �����ϱ⿡�� ������ �߻��� �� �����Ƿ� ������� ������ �������ؼ� ���
void ANelia::Jump()
{
	FVector LaunchVelocity;
	bool bXYOverride = false;
	bool bZOverride = false;

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if ((MovementStatus != EMovementStatus::EMS_Death))
	{
		bJump = true;
		//���� virtual void Jump() override; ��� ���� �Լ���
		//ex) //#include "GameFramework/Pawn.h"      
		//�̰� ���� �̻��ϰ� ��ӹ��� superŬ������ ĳ���Ϳ��� ������ �ٲ� �̾���
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

//GetCharacterMovement()�� ���Ͱ��� 0���� �ʱ�ȭ �� EMovementMode�� Flying���� ��ȯ���ش�.
void ANelia::ChangeModeToFly()
{
	if (EMovementMode::MOVE_Walking)
	{
		isClimb = true;
		GetCharacterMovement()->Velocity = FVector(0, 0, 0);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		
		//climb ���� ī�޶� ȸ���� ���� ���� ī�޶� ȸ���� ���
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}

//���� Ʈ���̽��� ���� �÷��̾� ���� ���� * 40.f �׸��� ���� ���� ���� ���� 70.f��ŭ ��ġ�� �����Ǵ� ���� �ִ��� Ȯ��
void ANelia::CanClimb()
{
	if (!isClimbUp)
	{
		FHitResult Hit;
		FVector Start = GetActorLocation();
		//TraceDistance�� ������ �� 40.f�� ����
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
		
		//���� ���� �� �ִϸ��̼� ����
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (isClimb && !onClimbLedge)
		{
			isClimbUp = true;
			isClimb = false;
			AnimInstance->Montage_Play(ClimbTop_Two, 1.2f);

			//Timer(������)
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

//spacebar�� ������ ���� ȣ��
void ANelia::StopJumping()
{
	bJump = false;
}

void ANelia::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CanClimb();

	if (MovementStatus == EMovementStatus::EMS_Death) return;

	//�ð� �� ���׹̳� �Һ� = ���׹̳��Һ��� * ������ 
	float DeltaStamina = StaminaDrainRate * DeltaTime;

	//���¹̳� ����(ESS)�� ���� ������ ����(EMS) ���� ��� ���¹̳��� ����� �� EMS_Sprint�� 500.f�� �ӵ��� �޸��� ������ �� EMS_Normal �⺻ �޸��� 300.f�� �ӵ��� �̵�
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
	check(PlayerInputComponent);//�Է¹��� Ű�� Ȯ��

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
					Targets.Remove(Enemy); //Ÿ���� ���� �� �迭���� ����

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
			
			//�Ͻ����� �޴��� ȭ�鿡 ���� ���� �̵����� ���ϵ��� �ϴ� �κ�
			MainPlayerController->bPauseMenuVisible;
			
			return true;
		}
	}
	return false;
}

//���콺�� �¿�� ������ ��� z���� ���� ������
void ANelia::Turn(float Value)
{
	if (CanMove(Value) && !isClimb)
	{
		AddControllerYawInput(Value);
	}
}

//���콺�� ���Ϸ� ������ ��� y���� ���� ������
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

	//�ӵ��� ���̷��� Velocity �� ����
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

//ESC, Q �Է� �� �ش� Pause �޴� â �˾�
void ANelia::ESCDown()
{
	bESCDown = true;

	if (MainPlayerController)
	{
		//�޴�â�� �������� ���� ���������� ���� �Լ�
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


//������ �ִϸ��̼��� ���߰� ���̷��� �����ӵ� ����
void ANelia::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

//�� Switch������ EMS_Sprinting ������ ���� �ӵ��� SprintingSpeed=375.f�� ���� �ְ� �� �ܿ��� Speed�� 280.f
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

			//���� Ű �Է��� ���������� ������ ���� ��� AttackMotionCount�� 1�� �����ϸ� 3���� ���� ������ ����
			//���������� ���� ��� �ִϸ��̼� ��Ƽ���̸� ���� ���� �ʱ�ȭ -> ù ��° ���� ��Ǹ� ����
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

		//MainPlayerController�� �Է� ���� Ű ���� pressSillNum�� ����
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
	//�� ���� ���� Ȯ��
	if (bOverlappingCombatSphere) //There is a enemy in combatsphere
	{
		if (targetIndex >= Targets.Num()) //Ÿ���ε����� �� Ÿ�� ���� �� �� �̻��̸� �ٽ� 0���� �ʱ�ȭ
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
		//���� combat target�� �̸��� �����Ͽ� �������
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

	// Ÿ�� ���� ����
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

//������ ���⿡ �Ҹ� ���
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

	//��Ȱ �� ��Ʈ�ѷ� �Ҵ�
	MainPlayerController->Possess(this);
	//�������Ʈ���� ��ġ ���� ������
	SetActorLocation(RespawnLocation);
	
	SetMovementStatus(EMovementStatus::EMS_Normal);

	APlayerCameraManager* playerCamera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	// ī�޶� ���̵� �ƿ�
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

//���� ���带 �޾ƿ��� ���� �� �̸��� CurrentLevel�� �Է� CurrentLevelName�� �Ű����� LevleName�� �ٸ��� LevleName���� �̵�
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

//e Ű �Է� �� ����
void ANelia::Interact()
{	
	if (bCanUseDialogue)
	{
		if (MainPlayerController->UserInterface != nullptr && (MainPlayerController->UserInterface->CurrentState !=3))
		{
			//ù ��ȭ ���۽ÿ��� CurrentState�� 0�̹Ƿ� Interact�� if���� ��� �������� ���ϰ� ������ �ǰ� 
			MainPlayerController->UserInterface->Interact();
		}

		//e ������ �� ù ��ȭ�� ��� ���⿡ �ִ� if���� �����ϰ� �ȴ�.
		if (!MainPlayerController->bDialogueVisible)
		{
			MainPlayerController->DisplayDialogue();
		}
	}
}
