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
	//Left Shift 키를 누르면 캐릭터 MaxWalkSpeed를 SprintingSpeed 값을 넣어 속도를 950.f만큼으로 이동하고 그렇지 않을 경우는 RunningSpeed 값을 넣어 
	//650.f 속도로 이동시킨다.
	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;

	
	bShiftKeyDown = false;
	bPickup = false;

	//열거형 EMovementStatus와 EStaminaStatus를 ESS_Normal로 초기화 시켜줌
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	//어디서든 편집 가능하고 블루프린터에서 읽고 쓰기가 가능한 float형 변수
	//StaminaDrainRate는 시간당 스테미나 소비량인 DeltaStamina 수식에 사용 DeltaStamina = StaminaDrainRate * DeltaTime;
	//MinSprintStamina는 아래 스위치 문에서 최소 스프린트 수치 이하일 경우 BelowMinimum(최소 이하) 상태로 변환하고 이 상태에서 스프린터키를 
	//누르게 되면 Exhausted 상태로 변화하여 달릴 수 없게됨
	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;	

	//
	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	
	bMovingForward = false;
	bMovingRight = false;

	bHasCombatTarget = false;
}

//게임 플레이 시 재정의 되는 부분
void ANelia::BeginPlay()
{
	Super::BeginPlay();
}

//Super::Jump는 Nelia 클래스의 부모 클래스에 있는 점프를 상속 받아서 불러오는 것이고 아래 bJump는 대쉬를 하는 도중에 점프를 하게 되어서 애니메이션 오류가 발생하여 
//부모 클래스에 있는 점프에 대한 정보를 변경하기에는 문제가 발생할 수 있으므로 헤더에서 점프를 override해서  다른 것은 부모의 점프를 상속 받고 bJump는 부모가 아닌 Nelia에 정의하여 사용하는 것
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

//매 프레임마다 호출되는 부분 헤더에서 UMainAnimInstance 클래스형 변수인 MainAnimInstance 생성
//만약 MainAnimInstance가 아니면 GetMesh(넬리아)에 있는 애니메이션 인스턴스를 가져오고 UMainAnimInstance로 형변환 후 MainAnimInstance에 대입
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


	//시간 당 스테미나 소비량 = 스테미나소비율 * 프레임 
	float DeltaStamina = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus)
	{
	//처음에는 ESS_Normal로 초기화 되어있고 120으로 초기화 되어있는 스테미나 - 시간 당 스테미나 소비량이 최소 스테미나 양보다 적으면 스태미나 상태를 최소 이하로 바꾸고
	//스프린팅 상태에서 이동키를 누를 때는 스프린팅 상태 그렇지 않을 경우에는 일반적인 상태? idle?
	//shift키 버튼을 안 누른 상태일 때 기존 스테미나 + 시간 당 스테미나 사용량 이 최대 스테미나 값보다 크면 스테미나를 최대 스테미나 값으로 초기화 시켜줌
	//최대 스테미나 값보다 크지 않을 경우에는 그냥 DeltaStamina 만큼 씩 더해줌 상태는 EMS_Normal
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

	//상태가 최소 이하일 때 Left Shift 키 입력 시 스테미나 - 시간 당 스테미나 소비량이 0.f보다 작거나 같으면 상태는 지침 상태로 주고 
	//스테미나를 0으로 초기화 이동 상태는 EMS_Normal로 둠 0.f보다 클 경우 기존 스테미나에서 시간 당 스테미나 소비량을 빼고 상태를 스프린터로 둔다.
	//Left Shift키를 누르지 않았을 때 스테미나 + 시간 당 스테미나 사용량이 최소 스프린터 가능 스테미나보다 크거나 같으면 스테미나 상태를 ESS_Normal로 두고
	//Stamina += DeltaStamina이고 Stamina + DeltaStamina <MinSprintStamina 일 때 동일하게 Stamina += DeltaStamina 상태는 EMS_Normal
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

	//스테미나 고갈 상태일 때 Left Shift 누르면 스테미나를 0.f로 초기화 키 입력을 안하고 있다면 스테미나 상태를 ESS_ExhaustedRecovering상태로 둔다.
	//키를 안 누르고 있기 때문에 Stamina += DeltaStamina; 계속해서 스테미나 증가 시키고 이동 상태는 EMS_Normal로 둠
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
	//ESS_ExhaustedRecovering 상태일 때는 최소 스프린팅 스테미나보다 크면 상태를 스테미나 상태를 일반으로 두고 매 프레임당 스테미나를 증가 시켜준다.

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

	//적을 바라보고 있고 전투대상이 있으면 실행되고 
	//LookAtYaw = 전투 대상의 위치를 받고 전투 대상과 플레이어의 회전 보간은 InterpRotation = 플레이어의 회전 정도, 전투대상 위치, 프레임당 , 보간 속도)를 
	//받아 SetActorRotation(InterpRotation)을 사용하여 보간을 한다. 즉 전투 대상의 위치를 받고 대상을 바라 보기 위한 회전 값을 받아서 SetActorRotation(InterpRotation)을 통해 보간한다.
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


//플레이어 컨트롤러가 있고 w,a,s,d 중 하나를 입력 중이고 현재 공격을 하고 있지 않을 때 Nelia 컨트롤러 방향 값을 받아 Rotation에 주고
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

//키를 누르고 있으면 컨트롤러가 1초안에 65도 회전 가능  
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