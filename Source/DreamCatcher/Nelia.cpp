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
#include "MainPlayerController.h"
#include "NeliaSaveGame.h"
#include "ItemStorage.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "DreamCatcherGameModeBase.h"
#include "GameFramework/GameMode.h"
#include "UserInterface.h"

// Sets default values
ANelia::ANelia()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//CameraBoom은 카메라와 플레이어를 이어주고 있는 것 처럼 보이는 빨간선 부모 컴포넌트인 Nelia의 Capsule Component에 부착되어 있고 그 사이 간격은 400.f
	//bUsePawnControllerRotation은 플레이어를 기준으로 도는 것을 가능하도록 하는 변수
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));   
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;							
	CameraBoom->bUsePawnControlRotation = true;						

	//Nelia의 캡슐 크기를 c++울 통해 조정 가능
	GetCapsuleComponent()->SetCapsuleSize(20.f, 76.f);				


	//FollowCamera를 카메라붐 끝 부분에 부착. FollowCamera는 이미 CameraBoom이 좌우로 돌기 때문에 같이 돌아갈 필요가 없어 false로 둠 단 CameraBoomd bUsePawnControlRotation을 false로두고
	//FollowCamera에서 true로 두면 Nelia를 기준으로 회전하지 않고 카메라를 기준으로 회전하기 때문에 캐릭터로부터 400.f 위치에서 홀로 돌아가게 됨
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));		 
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//키 입력 시 1초동안 65씩 회전 BaseTurnRate는 프로젝트 세팅 입력창에서 볼 수 있듯이 좌우 회전 비율이고 BaseLookupRate는 상하 회전 비율이다.
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	//카메라와 캐릭터가 동시 회전하는 걸 막는 역할 이렇게 설정하지 않으면 회전 시 캐릭터의 앞 모습을 절대 볼 수 없음.
	//Let that just affect the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; //bOrientRotationToMovement는 자동으로 캐릭터의 이동방향에 맞춰, 회전 보간을 해준다.
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f); //키 입력 시 540씩 회전함
	GetCharacterMovement()->JumpZVelocity = 650.f; //점프하는 힘
	GetCharacterMovement()->AirControl = 0.2f; //중력의 힘

	MaxHealth = 100.f;
	Health = 65.f;
	MaxStamina = 150.f;
	Stamina = 120.f;

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

	bHasCombatTarget = false;

	bRoll = false;
}

//게임 플레이 시 재정의 되는 부분
void ANelia::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());

	LoadGameNoSwitch();

	//if (MainPlayerController)
	//{
	//	MainPlayerController->GameModeOnly();
	//}
}
 
//부모 클래스에 있는 점프에 대한 정보를 변경하기에는 문제가 발생할 수 있으므로 헤더에서 점프를 override해서 다른 것은 부모의 점프를 상속 받고 bJump는 부모가 아닌 Nelia에 정의하여 사용하는 것
//if(MovementStatus != EMovementStatus::EMS_Death)는 공격 받은 이후 죽었을 때도 점프가 가능한 상황을 막는 코드 
void ANelia::Jump()
{
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if ((MovementStatus != EMovementStatus::EMS_Death))
	{
		Super::Jump();
		bJump = true;
	} 
}


void ANelia::StopJumping()
{
	Super::StopJumping();
	bJump = false;
}

void ANelia::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (MovementStatus == EMovementStatus::EMS_Death) return;


	//시간 당 스테미나 소비량 = 스테미나소비율 * 프레임 
	float DeltaStamina = StaminaDrainRate * DeltaTime;

	//스태미나 상태(ESS)에 따른 움직임 상태(EMS) 예를 들어 스태미나가 충분할 시 EMS_Sprint로 950.f의 속도로 달리고 부족할 시 EMS_Normal 기본 달리기 650.f의 속도로 이동
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
				SetMovementStatus(EMovementStatus::EMS_Normal);
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

	//적을 바라보고 있고 전투대상이 있으면 실행되고 
	//LookAtYaw = 전투 대상의 위치를 받고 전투 대상과 플레이어의 회전 보간은 InterpRotation = 플레이어의 회전 정도, 전투대상 위치, 프레임당 , 보간 속도)를 
	//받아 SetActorRotation(InterpRotation)을 사용하여 보간을 한다. 즉 전투 대상의 위치를 받고 대상을 바라 보기 위한 회전 값을 받아서 SetActorRotation(InterpRotation)을 통해 보간한다.
	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);


		SetActorRotation(InterpRotation);
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &ANelia::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &ANelia::ESCUp);

	PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &ANelia::Roll);

	PlayerInputComponent->BindAction("Pickup", IE_Pressed, this, &ANelia::PickupPress);
	PlayerInputComponent->BindAction("Pickup", IE_Released, this, &ANelia::PickupReleas);

	PlayerInputComponent->BindAxis("MoveForward", this, &ANelia::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANelia::MoveRight);

	PlayerInputComponent->BindAxis("turn", this, &ANelia::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ANelia::LookUp);
	PlayerInputComponent->BindAxis("TurnRate", this, &ANelia::TurnAtRate);
	PlayerInputComponent->BindAxis("LokkUpRate", this, &ANelia::LookUpAtRate);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ANelia::Interact);

	PlayerInputComponent->BindAction("keyUp", IE_Pressed, this, &ANelia::OnKeyUp);
	PlayerInputComponent->BindAction("KeyDown", IE_Pressed, this, &ANelia::OnKeyDown);
	//.bConsumeInput = false;
}

bool ANelia::CanMove(float Value)
{
	if (MainPlayerController)
	{
		return 
			(Value != 0.0f) && 
			(!bAttacking) &&
			(MovementStatus != EMovementStatus::EMS_Death) &&
			//일시정지 메뉴가 화면에 떴을 때는 이동하지 못하도록 하는 부분
			!MainPlayerController->bPauseMenuVisible;
	}
	return false;
}

void ANelia::Turn(float Value)
{
	if (CanMove(Value))
	{
		AddControllerYawInput(Value);
	}
}


void ANelia::LookUp(float Value)
{
	if (CanMove(Value))
	{
		AddControllerPitchInput(Value);
	}
}

//앞,뒤 이동 Value 값을 통해 설정해둔 1,-1 값을 받아 움직인다.
//사용자로부터 W,S키를 입력 받으면 호출되는 함수이고 컨트롤러가 있고 Value가 0이 아니고 공격, 죽은 상태가 아닐 경우 회전값과 방향을 받아 이동한다.
void ANelia::MoveForward(float Value)
{
	bMovingForward = false;

	if (CanMove(Value))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
		bMovingForward = true;
		if (isClimb)
		{
			if (isClimbLedge)
			{
				const FVector Direction_up = GetActorUpVector();
				AddMovementInput(Direction_up, Value);
			}
		}
	}
}


//좌,우 이동 위 MoveForward랑 구현 방식이 같음
void ANelia::MoveRight(float Value)
{
	bMovingRight = false;
	if (CanMove(Value))
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

//줍기 죽은 상태가 아니고 Item 클래스 자식인 Weapon 클래스에서 장착한 대상이 Nelia일 경우 해당 무기를 Weapon형태로 변환하고 장착 이후 nullptr로 값을 비워준다.
void ANelia::PickupPress()
{
	bPickup = true;

	if (MovementStatus == EMovementStatus::EMS_Death) return;

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (ActiveOverlappingItem && !bAttacking)
	{
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon)
		{
			Weapon->Equip(this);
			SetActiveOverlappingItem(nullptr);
		}
	}
	else if (EquippedWeapon)
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
}

void ANelia::PickupReleas()
{
	bPickup = false;
}

//ESC, Q 입력 시 해당 Pause 메뉴 창이 뜸 개발 과정에서는 ESC 입력 시 플레이가 중지되기 때문에 마지막에 Q는 빼줄 예정 
void ANelia::ESCDown()
{
	bESCDown = true;

	if (MainPlayerController)
	{
		//MainPlayerController에서 함수 선언 메뉴창이 떠있으면 끄고 켜져있으면 끄는 함수
		MainPlayerController->TogglePauseMenu();
	}
}
void ANelia::ESCUp()
{
	bESCDown = false;
}

//체력 증가 함수 
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

//체력 감소 함수
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


//CombatMontage를 1.2배 속도로 애니메이션을 실행하고 CombatMontage의 몽타주 섹션 Death 부분으로 이동
void ANelia::Die()
{
	MainPlayerController = Cast<AMainPlayerController>(GetController());
	if (MovementStatus == EMovementStatus::EMS_Death) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	//MainPlayerController->startplayer ///////////////////// 이거 character spawn 하려고 하던건데 음 startplayer을 어떻게 받아야 할지 모르겠음
	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.2f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
		
	}
	SetMovementStatus(EMovementStatus::EMS_Death);
	OnDeath();
}

//죽으면 애니메이션을 멈추고 스켈레톤 업데이트도 멈춘다.
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

//공격 중인 상태가 아니고 죽지 않았을 때 적 방향을 바라보고 Nelia의 AnimInstance를 가져옴
//case0부터 2까지 순서대로 시행하고 마지막 Attack3를 시행 후 다시 Attack1번부터 공격 모션을 수행함
void ANelia::Attack()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Death && EquippedWeapon)
	{
		bAttacking = true;
		SetInterpToEnemy(true);

		//MainPlayerController에 정의한 플레이어가 입력한 스킬 확인 함수에서 반환한 키 값을 pressSillNum에 대입
		int pressSkillNum = MainPlayerController->CheckInputKey();
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && CombatMontage)
		{
			int32 Section = AttackMotionCount;
			switch (Section)
			{
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 1.7f);
				AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
				break;
			case 1:
				AnimInstance->Montage_Play(CombatMontage, 1.7f);
				AnimInstance->Montage_JumpToSection(FName("Attack2"), CombatMontage);
				break;
			case 2:
				AnimInstance->Montage_Play(CombatMontage, 1.1f);
				AnimInstance->Montage_JumpToSection(FName("Attack3"), CombatMontage);
				break;
			default:
				break;
			}

			///////////// 플레이어가 입력한 스킬 키에 따라 스킬을 구현하는 부분 구현해야함 
			UBlueprintGeneratedClass* BringBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/Skill/MeteorSkill.MeteorSkill_C"));
			switch (pressSkillNum)
			{
			case 1:
				BringBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/DashAttack.WindAttack_C"));

				AnimInstance->Montage_Play(SkillMontage, 1.4f);
				AnimInstance->Montage_JumpToSection(FName("Skill4"), SkillMontage);

				UE_LOG(LogTemp, Warning, TEXT("skill1"));

				break;
			case 2:
				break;
			case 3:
				break;

			default:
				break;
			}
		}
		
		AttackMotionCount++;
		if (AttackMotionCount > 2)
		{
			AttackMotionCount = 0;
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

void ANelia::ResetCombo()
{
	AttackMotionCount = 0;
	saveAttack = false;
	bAttacking = false;
	UE_LOG(LogTemp, Warning, TEXT("reset"));
}

void ANelia::SaveComboAttack()
{
	if (saveAttack)
	{
		saveAttack = false;
		AttackEnd();
		Attack();
		UE_LOG(LogTemp, Warning, TEXT("savecombo"));
	}
}

//구르고 있지 않고 죽지 않았고 W,S 또는 A,D키를 누르고 있을 때 Nelia의 AnimInstance를 가져오고 RollMontage를 1.5배 속도로 실행한다.
void ANelia::Roll()
{
	if (!bRoll && MovementStatus != EMovementStatus::EMS_Death && (bMovingForward || bMovingRight))
	{
		bRoll = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (RollMontage && AnimInstance)
		{
			AnimInstance->Montage_Play(RollMontage, 1.5f);
			AnimInstance->Montage_JumpToSection(FName("Roll"), RollMontage);

			FTimerHandle WaitHandle;
			
			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
				{

					// 여기에 코드를 치면 된다.

				}), 5.f, false);
		}
	}
}

void ANelia::StopRoll()
{
	bRoll = false;
}

//장착한 무기에 SwingSound가 있으면 소리를 재생
void ANelia::PlaySwingSound()
{ 
	if (EquippedWeapon->SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}

//공격 시 적 쪽으로 향하게 함
void ANelia::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

//Nelia 체력 - 데미지량이 0보다 작거나 같으면 Health -= DamageAmount;를 해주고 Die()함수 호출 이후 Nelia에게 데미지를 준 대상이(DamageCauser)가 Enemy이면 유효 타겟을 false로 지정
//Health - DamageAmount <= 0.f가 아닐 경우 체력에서 데미지 만큼 빼주고 리턴
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
				Enemy->bHasValidTarget = false;
			}
		}
	}
	else
	{
		Health -= DamageAmount;
	}
	return DamageAmount;
}

void ANelia::OnDeath()
{
	MainPlayerController = Cast<AMainPlayerController>(GetController());
	MainPlayerController->UnPossess();
	APlayerCameraManager* playerCamera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);//StartCameraFade(0.f, 0.f, 1.f, FLinearColor::Black, false, true);
	playerCamera->StartCameraFade(0.f, 1.f, 3.f, FLinearColor::Black, false, true);
	FTimerHandle WaitHandle;
	MainPlayerController->DiedHUD->SetVisibility(ESlateVisibility::Visible);

	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
	{
			MainPlayerController->DiedHUD->SetVisibility(ESlateVisibility::Hidden);
			AGameMode* GameMode = Cast<AGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			GameMode->RestartGame();
	}), 3.1f, false);
}


//OverlappingActors라는 배열(순서대로 나열되는)을 만들고 배열에 아무것도 없고 MainPlayerController이면 적 체력바를 안 보이도록 한다.
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
	//OverlappingActors 배열 첫 번째 요소를 Enemy로 형 변환하여 ClosestEnemy에 넣고 대상 위치값과 최소거리를 구함
	//OverlappingActors 첫 번째 요소부터 순서대로 AEnemy 형태로 형변환 후 제일 가까운 적을 구하여 체력바를 볼 수 있도록 설정하고 공격 대상을 가장 가까운 적으로 지정한다.
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
		if (MainPlayerController)
		{
			MainPlayerController->DisplayEnemyHealthBar();
		}
		SetCombatTarget(ClosestEnemy);
		bHasCombatTarget = true;
	}
}

//현재 월드를 받아오고 현재 맵 이름을 CurrentLevel에 입력 CurrentLevelName과 매개변수 LevleName이 다르면 LevleName으로 이동
void ANelia::SwitchLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevel = World->GetMapName();

		FName CurrentLevelName(*CurrentLevel);
		if (CurrentLevelName != LevelName)
		{
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}


void ANelia::SaveGame()
{
	UNeliaSaveGame* SaveGameInstance = Cast<UNeliaSaveGame>(UGameplayStatics::CreateSaveGameObject(UNeliaSaveGame::StaticClass()));
	
	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	//현재 캐릭터가 있는 맵의 이름 StreamingLevelPrefix를 하니까 UEDPIE_0_ElvenRuins 이렇게 안 뜨고 
	//맵 이름인 ElvenRuins만 뜸
	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	SaveGameInstance->CharacterStats.LevelName = MapName;

	if (EquippedWeapon)
	{
		SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->Name;
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex); 
}

void ANelia::LoadGame(bool SetPosition)
{
	UNeliaSaveGame* LoadGameInstance = Cast<UNeliaSaveGame>(UGameplayStatics::CreateSaveGameObject(UNeliaSaveGame::StaticClass()));

	LoadGameInstance = Cast<UNeliaSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;

	if (WeaponStorage)
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
			WeaponToEquip->Equip(this);
		}
	}

	if (SetPosition)
	{
		SetActorLocation(LoadGameInstance->CharacterStats.Location);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
	}
	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;

	if (LoadGameInstance->CharacterStats.LevelName != TEXT(""))
	{
		FName LevelName(*LoadGameInstance->CharacterStats.LevelName);

		SwitchLevel(LevelName);
	}
}

void ANelia::LoadGameNoSwitch()
{
	UNeliaSaveGame* LoadGameInstance = Cast<UNeliaSaveGame>(UGameplayStatics::CreateSaveGameObject(UNeliaSaveGame::StaticClass()));

	LoadGameInstance = Cast<UNeliaSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;

	if (WeaponStorage)
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
			WeaponToEquip->Equip(this);
		}
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}

void ANelia::Interact()
{	
	if (MainPlayerController->UserInterface != nullptr)
	{
		MainPlayerController->UserInterface->Interact();
	}
	MainPlayerController->ToggleDialogue();
}

void ANelia::OnKeyUp()
{
	if (MainPlayerController->UserInterface != nullptr)
	{
		MainPlayerController->UserInterface->OnSelectUpOption();
	}
}

void ANelia::OnKeyDown()
{
	if (MainPlayerController->UserInterface != nullptr)
	{
		MainPlayerController->UserInterface->OnSelectDownOption();
	}
}