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

//#include "GameFramework/Pawn.h"      이거 쓰면 이상하게 상속받은 super클래스가 캐릭터에서 폰으로 바뀜 어이없음

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
	Health = 100.f;
	MaxStamina = 150.f;
	Stamina = 120.f;

	Level = 0;
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

	bHasCombatTarget = false;

	bRoll = false;
	TraceDistance = 40.f;

	isClimb = false;
	onClimbLedge = false;
	isClimbUp = false;
	
	checkPointCount = 0;

	wallLeftRight = 100.f;
	wallUpDown = 100.f;
}

//게임 플레이 시 재정의 되는 부분
void ANelia::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());

	//LoadGameNoSwitch();

	//if (MainPlayerController)
	//{
	//	MainPlayerController->GameModeOnly();
	//}
}
 
//부모 클래스에 있는 점프에 대한 정보를 변경하기에는 문제가 발생할 수 있으므로 헤더에서 점프를 재정의해서 사용
//죽은 상태가 아닐 경우 bool 변수에 true를 넣어줌
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
		//이거 쓰면 이상하게 상속받은 super클래스가 캐릭터에서 폰으로 바뀜 어이없음
		Super::Jump();

		if (isClimb)
		{
			LaunchVelocity.X = GetActorForwardVector().X * -500.f;
			LaunchVelocity.Y = GetActorForwardVector().Y * -500.f;
			LaunchVelocity.Z = 0.f;

			//space bar 입력 시 캐릭터를 일정 속도로 발사시킴
			LaunchCharacter(LaunchVelocity, bXYOverride, bZOverride);

			isClimb = false;

			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
			GetCharacterMovement()->bOrientRotationToMovement = true; 
			ChangeModeToFly();
		}
	} 
}

/// <summary>
/// GetCharacterMovement()의 EMovementMode가 걷기 모드인지 확인 맞을 경우
/// 벽을 타는지 확인 가능한 bool 변수인 isClimb에 true값을 주고
/// GetCharacterMovement()의 벡터값을 0으로 초기화 및 EMovementMode를 Flying모드로 변환해준다.
/// 
/// </summary>
void ANelia::ChangeModeToFly()
{
	if (EMovementMode::MOVE_Walking)
	{
		isClimb = true;
		GetCharacterMovement()->Velocity = FVector(0, 0, 0);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		
		//bOrientRotationToMovement는 캐릭터 이동 방향을 향하도록 회전한다.
		//카메라가 어느 방향을 향하든 캐릭터는 항상 자신이 움직이는 방향을 향하게 된다.
		//false이니까 그럼 카메라와 같은 방향을 향하게 되는 걸 의미???
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}

/// <summary>
/// Character 기준 정면 40 방향 대각선 위쪽 방향으로 135에 위치한 감지 선을 지정하고
/// 각 정면과 대각선 선에 감지되었는지를 bool 변수 내부에 전달 감지선은 각각 초록색과 빨간색으로 나타내주고
/// 만약 정면 선에 벽 충돌 시 onClimbLedge변수 내에 대각선 위 방향에 있는 선에 충돌한 벽이 있는지 
/// 여부를 나타내는 bool 값을 대입 -> ChangeModeToFly()함수로 이동을하게 되고 걷고 있는 상태라면 
/// 단 한 번 상태를 Flying 모드로 변환 감지되지 않았는데 iscClimb 변수가 true인 경우 false로 변환해주고
/// EMovemet 모드 상태를 Walking모드로 변환 추가적으로 카메라가 보는 방향과 관련 없이 캐릭터가 이동할 수 있도록 함
/// </summary>
void ANelia::CanClimb()
{
	if (!isClimbUp)
	{
		FHitResult Hit;
		FVector Start = GetActorLocation();
		FVector End = Start + (GetActorForwardVector() * TraceDistance);
		FVector arriveEnd = (Start + (GetActorForwardVector())+FVector(0.f, 0.f, 135.f) + (GetActorForwardVector() * TraceDistance));

		FCollisionQueryParams TraceParams;
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);
		bool bArriveHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, arriveEnd, ECC_Visibility, TraceParams);
		
		
		//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);
		//DrawDebugLine(GetWorld(), Start, arriveEnd, FColor::Red, false, 1, 0, 1);

		if (bHit)
		{
			onClimbLedge = bArriveHit;

			//SetActorRotation(FRotator(0.f, 0.0f, Hit.Normal.Z));
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

			//UE_LOG(LogTemp, Warning, TEXT("ledge"));

			isClimbUp = true;
			isClimb = false;
			AnimInstance->Montage_Play(ClimbTop_Two, 1.2f);

			//Timer(딜레이)
			FTimerHandle WaitHandle;

			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
				{
					//UE_LOG(LogTemp, Warning, TEXT("6"));
					isClimbUp = false;
					GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
					GetCharacterMovement()->bOrientRotationToMovement = true;
				}), 3.f, false);
		}
	}
}

//spacebar를 누르고 때면 호출되는 함수 bool변수에 false 값을 넣어줌 
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

	PlayerInputComponent->BindAction("Block", IE_Pressed, this, &ANelia::Block);
	PlayerInputComponent->BindAction("Block", IE_Released, this, &ANelia::BlockEnd);

	PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &ANelia::Roll);

	PlayerInputComponent->BindAction("Pickup", IE_Pressed, this, &ANelia::PickupPress);
	PlayerInputComponent->BindAction("Pickup", IE_Released, this, &ANelia::PickupReleas);

	PlayerInputComponent->BindAxis("turn", this, &ANelia::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ANelia::LookUp);
	//PlayerInputComponent->BindAxis("TurnRate", this, &ANelia::TurnAtRate);
	//PlayerInputComponent->BindAxis("LokkUpRate", this, &ANelia::LookUpAtRate);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ANelia::Interact);

	PlayerInputComponent->BindAction("SkillOne", IE_Pressed, this, &ANelia::Attack);
	PlayerInputComponent->BindAction("SkillTwo", IE_Pressed, this, &ANelia::Attack);
	PlayerInputComponent->BindAction("SkillThree", IE_Pressed, this, &ANelia::Attack);


	//.bConsumeInput = false;

	PlayerInputComponent->BindAxis("CameraZoom", this, &ANelia::CameraZoom);


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
	if (CanMove(Value))
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

//앞,뒤 이동 Value 값을 통해 설정해둔 1,-1 값을 받아 움직인다.
//사용자로부터 W,S키를 입력 받으면 호출되는 함수이고 컨트롤러가 있고 Value가 0이 아니고 
//공격 중인 상태 또는죽은 상태가 아닐 경우 회전값과 방향을 받아 이동한다.
void ANelia::MoveForward(float Value)
{
	bMovingForward = false;

	if (CanMove(Value) && !isClimb)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, Value);
		bMovingForward = true;
	}

	//속도를 더 높게 지정해주고 싶은데 해당 wallUpDown 값을 변환해줘도 속도가 변화되지 않음 
	if (isClimb)
	{
		wallUpDown = Value * 1000.f;
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Z);

		//test->>>> 원래 GetCharacterMovement()->Velocity = FVector(0, 0, 1000.f); 없었음
		if (Value != 0.0f)
		{
			GetCharacterMovement()->Velocity = FVector(0, 0, Value*150.f);
		}
		//위에꺼 안되면 아래꺼 
		//GetCharacterMovement()->Velocity = FVector(0, 0, Value*1000.f);

		//1.0f, -1.f 값이 최대 test 부분
		AddMovementInput(Direction, wallUpDown);

		//UE_LOG(LogTemp, Warning, TEXT("updown %f"), wallUpDown);
	}
}
//Log_Type 1
//UE_LOG(LogTemp, Warning, TEXT("updown %f"), wallUpDown);
//UE_LOG(LogTemp, Warning, TEXT("direction %s"), *Direction.ToString());

//좌,우 이동 위 MoveForward랑 구현 방식이 같음
void ANelia::MoveRight(float Value)
{
	if (CanMove(Value) && !isClimb)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, Value);
		bMovingRight = true;
	}

	bMovingRight = false;
	if (isClimb)											 
	{
		wallLeftRight = Value * 1000.f;
		//find out which way is forward
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

//키를 누르고 있으면 컨트롤러가 1초안에 65도 회전 가능 
//void ANelia::TurnAtRate(float Rate)
//{
//	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
//}
//
//void ANelia::LookUpAtRate(float Rate)
//{
//	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
//}

//죽은 상태가 아니고 Item에서 선언한 Collision volume 안에 감지된 아이템이 있고 공격중이 아니라면
//장착된 아이템을 weapon 형식으로 변환하여 클래스 변수 weapon에 넣어주고 해당 weapon을 장착 이후 SetActiveOverlappingitem에
//들어갔던 아이템들을 모두 비워줌 만약 무기를 이미 장착하고 있으면 combo attack에서 사용되는 saveattack을 true로 하고 
//공격 중이 아닐경우 Attack()함수를 호출함
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

	if (MainPlayerController->bDialogueVisible)
	{
		MainPlayerController->UserInterface->Interact();
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

void ANelia::Block()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Death)
	{
		bAttacking = true;
		SetInterpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && BlockMontage)
		{
			AnimInstance->Montage_Play(BlockMontage, 1.4f);
			AnimInstance->Montage_JumpToSection(FName("Parrying"), BlockMontage);
		}
	}
}

//건드림
void ANelia::BlockEnd()
{
	bAttacking = false;
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

void ANelia::AddExp()
{
	RandomInt = FMath::RandRange(10, 30);
	Exp += RandomInt;
	
	if (Exp >= MaxExp)
	{
		Level+= 1;
		Exp = 0.f;
		//UE_LOG(LogTemp, Warning, TEXT("MaxExp%d"), MaxExp);
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

//shift 키를 누르는 동안 호출되는 함수 
void ANelia::ShiftKeyDown()
{
	bShiftKeyDown = true;
}

//shift 키를 눌렀다 땠을 때 호출되는 함수 
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

//공격 중인 상태가 아니고 죽지 않았을 때 적 방향을 바라보고 Nelia의 AnimInstance를 가져옴
//case0부터 2까지 순서대로 시행하고 마지막 Attack3를 시행 후 다시 Attack1번부터 공격 모션을 수행함
//CombatMontage에서 공격 애니메이션마다 savecombo와 resetcombo를 지정해주었고 savecombo와 reset combo 사이에 공격 키를 계속해서 누르면 
//1번, 2번, 3번 기본 공격 동작을 수행하지만 시간을 두고 기본 공격 키 입력 시 계속해서 reset combo에서 공격모션이 0으로 초기화되기 때문에
//1번 공격만 하도록 구현
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
				AnimInstance->Montage_Play(CombatMontage, 1.4f);
				AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
				break;
			case 1:
				AnimInstance->Montage_Play(CombatMontage, 1.4f);
				AnimInstance->Montage_JumpToSection(FName("Attack2"), CombatMontage);
				break;
			case 2:
				AnimInstance->Montage_Play(CombatMontage, 1.4f);
				AnimInstance->Montage_JumpToSection(FName("Attack4"), CombatMontage);
				break;
			case 3:
				AnimInstance->Montage_Play(CombatMontage, 1.4f);
				AnimInstance->Montage_JumpToSection(FName("Attack3"), CombatMontage);
				break;
			default:
				break;
			}

			///////////// 플레이어가 입력한 스킬 키에 따라 스킬을 구현하는 부분 구현해야함 
			//UBlueprintGeneratedClass* BringBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/Skill/MeteorSkill.MeteorSkill_C"));
			if (AnimInstance && SkillMontage)
			{
				//UE_LOG(LogTemp, Warning, TEXT("pressSkillNum"));

				switch (pressSkillNum)
				{
				case 1:
					//BringBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/DashAttack.WindAttack_C"));
					AnimInstance->Montage_Play(SkillMontage, 0.9f);
					AnimInstance->Montage_JumpToSection(FName("Skill1"), SkillMontage);
					break;
				case 2:
					AnimInstance->Montage_Play(SkillMontage, 1.f);
					AnimInstance->Montage_JumpToSection(FName("Skill2"), SkillMontage);
					break;
				case 3:
					AnimInstance->Montage_Play(SkillMontage, 0.2f);
					AnimInstance->Montage_JumpToSection(FName("Skill3"), SkillMontage);
					break;
				default:
					break;
				}
			}
		}

		
		AttackMotionCount++;
		if (AttackMotionCount > 3)
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
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

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
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(SkillMontage, 1.f);
			AnimInstance->Montage_JumpToSection(FName("Block"), SkillMontage);
		}
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
	//UE_LOG(LogTemp, Warning, TEXT("SaveGame"));
	//UNeliaSaveGame* SaveGameInstance = Cast<UNeliaSaveGame>(UGameplayStatics::CreateSaveGameObject(UNeliaSaveGame::StaticClass()));
	//
	//SaveGameInstance->CharacterStats.Health = Health;
	//SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	//SaveGameInstance->CharacterStats.Stamina = Stamina;
	//SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	//SaveGameInstance->CharacterStats.Level = Level;
	//SaveGameInstance->CharacterStats.Exp = Exp;
	////SaveGameInstance->CharacterStats.MaxExp = MaxExp;

	//SaveGameInstance->CharacterStats.Location = GetActorLocation();
	//SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	////현재 캐릭터가 있는 맵의 이름 StreamingLevelPrefix를 하니까 UEDPIE_0_ElvenRuins 이렇게 안 뜨고 
	////맵 이름인 ElvenRuins만 뜸
	//FString MapName = GetWorld()->GetMapName();
	//MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	//SaveGameInstance->CharacterStats.LevelName = MapName;

	//if (EquippedWeapon)
	//{
	//	SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->Name;
	//}

	//UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex); 
}

void ANelia::LoadGame(bool SetPosition)
{
	//UE_LOG(LogTemp, Warning, TEXT("LoadGame"));
	//UNeliaSaveGame* LoadGameInstance = Cast<UNeliaSaveGame>(UGameplayStatics::CreateSaveGameObject(UNeliaSaveGame::StaticClass()));

	//LoadGameInstance = Cast<UNeliaSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	//Health = LoadGameInstance->CharacterStats.Health;
	//MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	//Stamina = LoadGameInstance->CharacterStats.Stamina;
	//MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	//Level = LoadGameInstance->CharacterStats.Level;
	//Exp = LoadGameInstance->CharacterStats.Exp;
	////MaxExp = LoadGameInstance->CharacterStats.MaxExp;

	//if (WeaponStorage)
	//{
	//	AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
	//	if (Weapons)
	//	{
	//		FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

	//		AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
	//		WeaponToEquip->Equip(this);
	//	}
	//}

	//if (SetPosition)
	//{
	//	SetActorLocation(LoadGameInstance->CharacterStats.Location);
	//	SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
	//}
	//SetMovementStatus(EMovementStatus::EMS_Normal);
	//GetMesh()->bPauseAnims = false;
	//GetMesh()->bNoSkeletonUpdate = false;

	//if (LoadGameInstance->CharacterStats.LevelName != TEXT(""))
	//{
	//	FName LevelName(*LoadGameInstance->CharacterStats.LevelName);

	//	SwitchLevel(LevelName);
	//}
}

void ANelia::LoadGameNoSwitch()
{
	/*UNeliaSaveGame* LoadGameInstance = Cast<UNeliaSaveGame>(UGameplayStatics::CreateSaveGameObject(UNeliaSaveGame::StaticClass()));

	LoadGameInstance = Cast<UNeliaSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Level = LoadGameInstance->CharacterStats.Level;
	Exp = LoadGameInstance->CharacterStats.Exp;
	MaxExp = LoadGameInstance->CharacterStats.MaxExp;

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
	GetMesh()->bNoSkeletonUpdate = false;*/
}

// 사용자가 e키를 입력했을 경우 수행되는 함수
void ANelia::Interact()
{	
	/// <summary>
	/// bCanUseDialogue는 레벨 블루프린트에서 npc 콜리전에 플레이어가 검출되었을 때만 즉 맵 전체에서 
	/// E키를 눌렀을 때 대화창이 뜨는 것이 아니라 npc 근처에 있을 때만 올라오도록 해야하기 때문에
	/// bool변수 bCanUseDialogue를 통해 확인
	/// </summary>
	if (bCanUseDialogue)
	{
		/// <summary>
		/// MainPlayerController에서 맨 위에서 UserInterface를 넣어주는데 생성자이기 
		/// 때문에 거의 확실하게 입력됨 CurrentState !=3이 의미는 대화시스템 중에 대사를 선택할 수 있는 경우가 있는데
		/// 해당 부분이 UserInterface.cpp에서 CurrentState 3인 경우 만족하도록 코드가 구성되어 있는데 
		/// 대화창이 떠있는 경우 e키를 한 번더 누르면 예를들어 한 대화 즉 npc와 플레이어가 대화가 있고 
		/// 해당 npc 대사에 대한 플레이어의 선택 대사가 가능할 경우 e키를 입력하면 다음 대화로 넘어가버리는 것을 방지
		/// </summary>
		if (MainPlayerController->UserInterface != nullptr && (MainPlayerController->UserInterface->CurrentState !=3))
		{
			//UE_LOG(LogTemp, Warning, TEXT("nelia interact in"));
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
