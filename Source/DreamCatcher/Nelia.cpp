// Fill out your copyright notice in the Description page of Project Settings.


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
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Enemy.h"
#include "MainAnimInstance.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ANelia::ANelia()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//������� ��𼭵� �� �� ������ ������ �� ���� ��������Ϳ��� ���� �б⸸ �����ϸ� private������ �����Ϳ��� �� �� �ִ� USpringArm Ŭ������ ������ CameraBoom�� ����
	//�ǹ��� 1. �� ���� public���� ������ ���� meta = (AllowPrivateAcess = "true")�� ������?
	//CameraBoom�� ��������Ϳ��� �� �� �ֵ��� ī�޶�� Nelia ���̸� �������ְ� �ִ� ���� ���̸� TEXT - CameraBoom���� �����ϸ� ��������Ϳ��� Ȯ���� �� �ִ�.
	//SetupAttachment(GetRootComponent())�� CameraBoom�� �ڽ� ������Ʈ�� �߰��ϰڴٴ� �ǹ��̰� ��������Ϳ��� ���� Nelia�� ĸ��������Ʈ(==GetRootComponent)�� �����Ǿ� �ִ� ���� �� �� �ִ�
	//CameraBoom�� ���̴� 400.f�̰� bUsePawnControlRotation�� ���콺 �̵����� ȭ�� �����¿� �̵��� �����ϵ��� �Ѵ�.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));   
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;							
	CameraBoom->bUsePawnControlRotation = true;						

	GetCapsuleComponent()->SetCapsuleSize(20.f, 76.f);				//Nelia�� ĸ�� ũ�⸦ c++�� ���� ���� ����

	//������� ��𼭵� �� �� ������ ������ �� ���� ��������Ϳ��� ���� �б⸸ �����ϸ� private������ �����Ϳ��� �� �� �ִ� UCameraComponent Ŭ������ ������ FollowCamera�� ����
	//FollowCamera�� ī�޶�� ���� �κ� ������Ų��. FollowCamera�� �̹� CameraBoom�� �¿�� ���� ���� ���ư� �ʿ䰡 ���� ������ false�� �д�. �� CameraBoomdml bUsePawnControlRotation�� false�εΰ�
	//FollowCamera���� true�� �θ� Nelia�� �������� ȸ������ �ʰ� ī�޶� �������� ȸ���ϱ� ������ ĳ���ͷκ��� 400.f ��ġ���� Ȧ�� ���ư��� �Ǿ� �׷��� �����ϸ� �ȵȴ�.
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));		 
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//������� ��𼭵� �� �� ������ ������ �� ���� ��������Ϳ��� ���� �б⸸ �����ϵ��� ����
	//Ű �Է� �� 1�ʵ��� 65�� ȸ�� BaseTurnRate�� ������Ʈ ���� �Է�â���� �� �� �ֵ��� �¿� ȸ�� �����̰� BaseLookupRate�� ���� ȸ�� �����̴�.
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	//Don't rotate when the controller rotates //ī�޶�� ĳ���Ͱ� ���� ȸ���ϴ� �� ���� ����?
	//Let that just affect the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; //bOrientRotationToMovement�� �ڵ����� ĳ������ �̵����⿡ ����, ȸ�� ������ ���ش�.
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f); //Ű �Է� �� 540�� ȸ����??? 
	GetCharacterMovement()->JumpZVelocity = 650.f; //�����ϴ� ��
	GetCharacterMovement()->AirControl = 0.2f; //�߷��� ��

	//������� max�� ��� �ν��Ͻ��� �������� ������ �� ���� ������ �����ϵ��� ���� �׸��� ��������Ϳ��� �б⸸ �����ϴ�.
	//Health�� Stamina�� ��𼭵� ���� �����ϰ� ��������Ϳ��� �а� ���� ���� �����ϴ�.
	MaxHealth = 100.f;
	Health = 65.f;
	MaxStamina = 150.f;
	Stamina = 120.f;

	//��𼭵� ���� �����ϰ� ��������Ϳ��� �а� ���Ⱑ �����ϴ�.
	//Left Shift Ű�� ������ ĳ���� MaxWalkSpeed�� SprintingSpeed ���� �־� �ӵ��� 950.f��ŭ���� �̵��ϰ� �׷��� ���� ���� RunningSpeed ���� �־� 
	//650.f �ӵ��� �̵���Ų��.
	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;

	
	bShiftKeyDown = false;
	bPickup = false;

	//������ EMovementStatus�� EStaminaStatus�� ESS_Normal�� �ʱ�ȭ ������
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	//��𼭵� ���� �����ϰ� ��������Ϳ��� �а� ���Ⱑ ������ float�� ����
	//StaminaDrainRate�� �ð��� ���׹̳� �Һ��� DeltaStamina ���Ŀ� ��� DeltaStamina = StaminaDrainRate * DeltaTime;
	//MinSprintStamina�� �Ʒ� ����ġ ������ �ּ� ������Ʈ ��ġ ������ ��� BelowMinimum(�ּ� ����) ���·� ��ȯ�ϰ� �� ���¿��� ��������Ű�� 
	//������ �Ǹ� Exhausted ���·� ��ȭ�Ͽ� �޸� �� ���Ե�
	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;	

	//
	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	
	bMovingForward = false;
	bMovingRight = false;

	bHasCombatTarget = false;
}

//���� �÷��� �� ������ �Ǵ� �κ�
void ANelia::BeginPlay()
{
	Super::BeginPlay();
}

//Super::Jump�� Nelia Ŭ������ �θ� Ŭ������ �ִ� ������ ��� �޾Ƽ� �ҷ����� ���̰� �Ʒ� bJump�� �뽬�� �ϴ� ���߿� ������ �ϰ� �Ǿ �ִϸ��̼� ������ �߻��Ͽ� 
//�θ� Ŭ������ �ִ� ������ ���� ������ �����ϱ⿡�� ������ �߻��� �� �����Ƿ� ������� ������ override�ؼ�  �ٸ� ���� �θ��� ������ ��� �ް� bJump�� �θ� �ƴ� Nelia�� �����Ͽ� ����ϴ� ��
void ANelia::Jump()
{
	Super::Jump();
	bJump = true;
}

void ANelia::StopJumping()
{
	Super::StopJumping();
	bJump = false;
}

//�� �����Ӹ��� ȣ��Ǵ� �κ� ������� UMainAnimInstance Ŭ������ ������ MainAnimInstance ����
//���� MainAnimInstance�� �ƴϸ� GetMesh(�ڸ���)�� �ִ� �ִϸ��̼� �ν��Ͻ��� �������� UMainAnimInstance�� ����ȯ �� MainAnimInstance�� ����
void ANelia::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!MainAnimInstance)
	{
		MainAnimInstance = Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance());
	}

	/*if (MainAnimInstance->bIsInAir)
	{
		DashDistance = 50.f;
		UE_LOG(LogTemp, Warning, TEXT("50"));

	}
	else
	{
		DashDistance = 3000.f;
		UE_LOG(LogTemp, Warning, TEXT("3000"));

	}*/

	//if (MovementStatus == EMovementStatus::EMS_Death) return;


	//�ð� �� ���׹̳� �Һ� = ���׹̳��Һ��� * ������ 
	float DeltaStamina = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus)
	{
	//ó������ ESS_Normal�� �ʱ�ȭ �Ǿ��ְ� 120���� �ʱ�ȭ �Ǿ��ִ� ���׹̳� - �ð� �� ���׹̳� �Һ��� �ּ� ���׹̳� �纸�� ������ ���¹̳� ���¸� �ּ� ���Ϸ� �ٲٰ�
	//�������� ���¿��� �̵�Ű�� ���� ���� �������� ���� �׷��� ���� ��쿡�� �Ϲ����� ����? idle?
	//shiftŰ ��ư�� �� ���� ������ �� ���� ���׹̳� + �ð� �� ���׹̳� ��뷮 �� �ִ� ���׹̳� ������ ũ�� ���׹̳��� �ִ� ���׹̳� ������ �ʱ�ȭ ������
	//�ִ� ���׹̳� ������ ũ�� ���� ��쿡�� �׳� DeltaStamina ��ŭ �� ������ ���´� EMS_Normal
	case EStaminaStatus::ESS_Normal:
		if (bShiftKeyDown)
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
				{
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
			}
		}

		else //shift key up
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

	//���°� �ּ� ������ �� Left Shift Ű �Է� �� ���׹̳� - �ð� �� ���׹̳� �Һ��� 0.f���� �۰ų� ������ ���´� ��ħ ���·� �ְ� 
	//���׹̳��� 0���� �ʱ�ȭ �̵� ���´� EMS_Normal�� �� 0.f���� Ŭ ��� ���� ���׹̳����� �ð� �� ���׹̳� �Һ��� ���� ���¸� �������ͷ� �д�.
	//Left ShiftŰ�� ������ �ʾ��� �� ���׹̳� + �ð� �� ���׹̳� ��뷮�� �ּ� �������� ���� ���׹̳����� ũ�ų� ������ ���׹̳� ���¸� ESS_Normal�� �ΰ�
	//Stamina += DeltaStamina�̰� Stamina + DeltaStamina <MinSprintStamina �� �� �����ϰ� Stamina += DeltaStamina ���´� EMS_Normal
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

	//���׹̳� �� ������ �� Left Shift ������ ���׹̳��� 0.f�� �ʱ�ȭ Ű �Է��� ���ϰ� �ִٸ� ���׹̳� ���¸� ESS_ExhaustedRecovering���·� �д�.
	//Ű�� �� ������ �ֱ� ������ Stamina += DeltaStamina; ����ؼ� ���׹̳� ���� ��Ű�� �̵� ���´� EMS_Normal�� ��
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
	//ESS_ExhaustedRecovering ������ ���� �ּ� �������� ���׹̳����� ũ�� ���¸� ���׹̳� ���¸� �Ϲ����� �ΰ� �� �����Ӵ� ���׹̳��� ���� �����ش�.

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

	//���� �ٶ󺸰� �ְ� ��������� ������ ����ǰ� 
	//LookAtYaw = ���� ����� ��ġ�� �ް� ���� ���� �÷��̾��� ȸ�� ������ InterpRotation = �÷��̾��� ȸ�� ����, ������� ��ġ, �����Ӵ� , ���� �ӵ�)�� 
	//�޾� SetActorRotation(InterpRotation)�� ����Ͽ� ������ �Ѵ�. �� ���� ����� ��ġ�� �ް� ����� �ٶ� ���� ���� ȸ�� ���� �޾Ƽ� SetActorRotation(InterpRotation)�� ���� �����Ѵ�.
	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);


		SetActorRotation(InterpRotation);
	}

	/*if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}*/
}

FRotator ANelia::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

// Called to bind functionality to input
void ANelia::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);					
	check(PlayerInputComponent);											//�Է¹��� Ű�� Ȯ��


	//�Է� ���� Ű�� ���� �ش� �̸��� �´� �Լ� ȣ��? JUMP, �̵�, ĳ���� ȸ���� 
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ANelia::Jump);				
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ANelia::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ANelia::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ANelia::ShiftKeyUp);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ANelia::Dash);

	PlayerInputComponent->BindAction("Pickup", IE_Pressed, this, &ANelia::PickupPress);
	PlayerInputComponent->BindAction("Pickup", IE_Released, this, &ANelia::PickupReleas);

	PlayerInputComponent->BindAxis("MoveForward", this, &ANelia::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANelia::MoveRight);

	PlayerInputComponent->BindAxis("turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ANelia::TurnAtRate);
	PlayerInputComponent->BindAxis("LokkUpRate", this, &ANelia::LookUpAtRate);
}


//�÷��̾� ��Ʈ�ѷ��� �ְ� w,a,s,d �� �ϳ��� �Է� ���̰� ���� ������ �ϰ� ���� ���� �� Nelia ��Ʈ�ѷ� ���� ���� �޾� Rotation�� �ְ�
//
void ANelia::MoveForward(float Value)
{
	bMovingForward = false;
	if ((Controller != nullptr) && (Value != 0.0f) && (!bAttacking))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		bMovingForward = true;
	}
}

void ANelia::MoveRight(float Value)
{
	bMovingRight = false;
	if ((Controller != nullptr) && (Value != 0.0f) && (!bAttacking)) //&& (MovementStatus != EMovementStatus::EMS_Death))
	{
		//find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

		bMovingRight = true;
	}
}

//Ű�� ������ ������ ��Ʈ�ѷ��� 1�ʾȿ� 65�� ȸ�� ����  
void ANelia::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ANelia::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ANelia::PickupPress()
{
	bPickup = true;
	if (ActiveOverlappingItem)
	{
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon)
		{
			Weapon->Equip(this);
			SetActiveOverlappingItem(nullptr);
		}
	}
	if (EquippedWeapon)
	{
		Attack();
	}
}

void ANelia::PickupReleas()
{
	bPickup = false;
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

void ANelia::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Death) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}
	SetMovementStatus(EMovementStatus::EMS_Death);
}

void ANelia::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
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

void ANelia::Attack()
{
	if (!bAttacking) //&& MovementStatus != EMovementStatus::EMS_Death)
	{
		bAttacking = true;
		SetInterpToEnemy(true);

		if (MainAnimInstance && CombatMontage)
		{
			int32 Section = combatCount;
			switch (Section)
			{
			case 0:
				MainAnimInstance->Montage_Play(CombatMontage, 2.4f);
				MainAnimInstance->Montage_JumpToSection(FName("Attack1"), CombatMontage);
				break;
			case 1:
				MainAnimInstance->Montage_Play(CombatMontage, 2.3f);
				MainAnimInstance->Montage_JumpToSection(FName("Attack2"), CombatMontage);
				break;
			case 2:
				MainAnimInstance->Montage_Play(CombatMontage, 1.5f);
				MainAnimInstance->Montage_JumpToSection(FName("Attack3"), CombatMontage);
			default:
				;
			}
		}
		combatCount++;
		if (combatCount > 2)
		{
			combatCount = 0;
		}
	}
}

void ANelia::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);
	if (bPickup)
	{
		Attack();
	}
}
////////
void ANelia::Dash()
{
	const FVector ForwardDir = this->GetActorRotation().Vector();


	if (DashMontage && bDash)
	{
		//LaunchCharacter(ForwardDir * DashDistance, true, true);
		//PlayAnimMontage(DashMontage);

		MainAnimInstance->Montage_Play(CombatMontage);
		MainAnimInstance->Montage_JumpToSection(FName("roll"), CombatMontage);
		bDash = false;

		GetWorldTimerManager().SetTimer(DashTimer, this, &ANelia::CanDash, 2.f);
	}
}

void ANelia::CanDash()
{
	bDash = true;
	GetWorldTimerManager().ClearTimer(DashTimer);
}

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
	if (Health - DamageAmount <= 0.f)
	{
		Health -= DamageAmount;
		Die();
		if (DamageCauser)
		{
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			if (Enemy)
			{
			//	Enemy->bHasValidTarget = false;
			}
		}
	}
	else
	{
		Health -= DamageAmount;
	}
	return DamageAmount;
}