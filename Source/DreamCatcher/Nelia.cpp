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

// Sets default values
ANelia::ANelia()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Camera boom(충격이 있는 경우 플레이어를 향해 당김)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));   
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;							//카메라가 Neila와 400떨어진 위치를 고정으로 따라옴
	CameraBoom->bUsePawnControlRotation = true;						//컨트롤러에 따라 CameraArm 회전

	
	GetCapsuleComponent()->SetCapsuleSize(20.f, 76.f);				//Nelia의 캡슐 높이와 반지름 지정

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));		 
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	//Attach the camera to the end of the boom and let the boom adjust to match
	//the controller orientation
	FollowCamera->bUsePawnControlRotation = false;

	//Set our turn rates for input
	//키 입력 시 1초동안 65씩 회전
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	//Don't rotate when the controller rotates //카메라와 캐릭터가 동시 회전하는 걸 막는 역할?
	//Let that just affect the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; //bOrientRotationToMovement는 자동으로 캐릭터의 이동방향에 맞춰, 회전 보간을 해준다.
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f); //키 입력 시 540씩 회전함??? 
	GetCharacterMovement()->JumpZVelocity = 650.f; //점프하는 힘
	GetCharacterMovement()->AirControl = 0.2f; //중력의 힘

	MaxHealth = 100.f;
	Health = 65.f;
	MaxStamina = 150.f;
	Stamina = 120.f;
	Coins = 0;

	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;

	bShiftKeyDown = false;
	bPickup = false;

	//Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;	

	bMovingForward = false;
	bMovingRight = false;

}

// Called when the game starts or when spawned
void ANelia::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ANelia::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



	//if (MovementStatus == EMovementStatus::EMS_Death) return;

	float DeltaStamina = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus)
	{
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

}

// Called to bind functionality to input
void ANelia::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);					//키보드 입력 값을 전달받는 폰의 함수
	check(PlayerInputComponent);											//입력받은 키를 확인


	//입력 받은 키에 따라 해당 이름에 맞는 함수 호출? JUMP, 이동, 캐릭터 회전등 
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ANelia::Jump);				
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

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

void ANelia::MoveForward(float Value)
{
	bMovingForward = false;
	if ((Controller != nullptr) && (Value != 0.0f) && (!bAttacking)) //&& (MovementStatus != EMovementStatus::EMS_Death))
	{
		//find out which way is forward
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

//키를 누르고 있으면 컨트롤러가 1초안에 65도 회전 가능   ///getworld?
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
		//SetInterpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage)
		{
			int32 Section = combatCount;
			switch (Section)
			{
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 2.4f);
				AnimInstance->Montage_JumpToSection(FName("Attack1"), CombatMontage);
				break;
			case 1:
				AnimInstance->Montage_Play(CombatMontage, 2.3f);
				AnimInstance->Montage_JumpToSection(FName("Attack2"), CombatMontage);
				break;
			case 2:
				AnimInstance->Montage_Play(CombatMontage, 1.5f);
				AnimInstance->Montage_JumpToSection(FName("Attack3"), CombatMontage);
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
	//SetInterpToEnemy(false);
	if (bPickup)
	{
		Attack();
	}
}
////////
void ANelia::Dash()
{
	const FVector ForwardDir = this->GetActorRotation().Vector();
	LaunchCharacter(ForwardDir * DashDistance, true, true);
	if (DashMontage && bDash)
	{
		PlayAnimMontage(DashMontage, 1, NAME_None);
		bDash = false;
	}
}
