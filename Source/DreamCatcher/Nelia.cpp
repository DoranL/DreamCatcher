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

	//헤더에서 어디서든 볼 수 있지만 편집할 수 없고 블루프린터에서 오직 읽기만 가능하며 private일지라도 에디터에서 볼 수 있는 USpringArm 클래스형 변수인 CameraBoom을 선언
	//의문점 1. 왜 굳이 public으로 선언해 놓고 meta = (AllowPrivateAcess = "true")를 했을까?
	//CameraBoom은 블루프린터에서 볼 수 있듯이 카메라와 Nelia 사이를 연결해주고 있는 빨간 선이며 TEXT - CameraBoom값을 변경하면 블루프린터에서 확인할 수 있다.
	//SetupAttachment(GetRootComponent())는 CameraBoom을 자식 컴포넌트로 추가하겠다는 의미이고 블루프린터에서 보면 Nelia의 캡슐컴포넌트(==GetRootComponent)에 부착되어 있는 것을 볼 수 있다
	//CameraBoom의 길이는 400.f이고 bUsePawnControlRotation은 마우스 이동으로 화면 상하좌우 이동을 가능하도록 한다.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));   
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;							
	CameraBoom->bUsePawnControlRotation = true;						

	GetCapsuleComponent()->SetCapsuleSize(20.f, 76.f);				//Nelia의 캡슐 크기를 c++울 통해 조정 가능

	//헤더에서 어디서든 볼 수 있지만 편집할 수 없고 블루프린터에서 오직 읽기만 가능하며 private일지라도 에디터에서 볼 수 있는 UCameraComponent 클래스형 변수인 FollowCamera를 선언
	//FollowCamera를 카메라붐 끝에 부분 부착시킨다. FollowCamera는 이미 CameraBoom이 좌우로 돌면 같이 돌아갈 필요가 없기 때문에 false로 둔다. 단 CameraBoomdml bUsePawnControlRotation을 false로두고
	//FollowCamera에서 true로 두면 Nelia를 기준으로 회전하지 않고 카메라를 기준으로 회전하기 때문에 캐릭터로부터 400.f 위치에서 홀로 돌아가게 되어 그렇게 설정하면 안된다.
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));		 
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//헤더에서 어디서든 볼 수 있지만 편집할 수 없고 블루프린터에서 오직 읽기만 가능하도로 설정
	//키 입력 시 1초동안 65씩 회전 BaseTurnRate는 프로젝트 세팅 입력창에서 볼 수 있듯이 좌우 회전 비율이고 BaseLookupRate는 상하 회전 비율이다.
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

	//헤더에서 max의 경우 인스턴스가 공통으로 가져야 할 값의 편집이 가능하도록 설정 그리고 블루프린터에서 읽기만 가능하다.
	//Health와 Stamina는 어디서든 편집 가능하고 블루프린터에서 읽고 쓰는 것이 가능하다.
	MaxHealth = 100.f;
	Health = 65.f;
	MaxStamina = 150.f;
	Stamina = 120.f;

	//어디서든 편집 가능하고 블루프린터에서 읽고 쓰기가 가능하다.
	//
	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;

	bShiftKeyDown = false;
	bPickup = false;

	//Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;	

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	bMovingForward = false;
	bMovingRight = false;

}

// Called when the game starts or when spawned
void ANelia::BeginPlay()
{
	Super::BeginPlay();
}

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

// Called every frame
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
	Super::SetupPlayerInputComponent(PlayerInputComponent);					//키보드 입력 값을 전달받는 폰의 함수
	check(PlayerInputComponent);											//입력받은 키를 확인


	//입력 받은 키에 따라 해당 이름에 맞는 함수 호출? JUMP, 이동, 캐릭터 회전등 
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