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

//#include "GameFramework/Pawn.h"      �̰� ���� �̻��ϰ� ��ӹ��� superŬ������ ĳ���Ϳ��� ������ �ٲ� ���̾���

// Sets default values
ANelia::ANelia()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//CameraBoom�� ī�޶�� �÷��̾ �̾��ְ� �ִ� �� ó�� ���̴� ������ �θ� ������Ʈ�� Nelia�� Capsule Component�� �����Ǿ� �ְ� �� ���� ������ 400.f
	//bUsePawnControllerRotation�� �÷��̾ �������� ���� ���� �����ϵ��� �ϴ� ����
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));   
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;							
	CameraBoom->bUsePawnControlRotation = true;						

	//Nelia�� ĸ�� ũ�⸦ c++�� ���� ���� ����
	GetCapsuleComponent()->SetCapsuleSize(20.f, 76.f);				


	//FollowCamera�� ī�޶�� �� �κп� ����. FollowCamera�� �̹� CameraBoom�� �¿�� ���� ������ ���� ���ư� �ʿ䰡 ���� false�� �� �� CameraBoomd bUsePawnControlRotation�� false�εΰ�
	//FollowCamera���� true�� �θ� Nelia�� �������� ȸ������ �ʰ� ī�޶� �������� ȸ���ϱ� ������ ĳ���ͷκ��� 400.f ��ġ���� Ȧ�� ���ư��� ��
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));		 
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//Ű �Է� �� 1�ʵ��� 65�� ȸ�� BaseTurnRate�� ������Ʈ ���� �Է�â���� �� �� �ֵ��� �¿� ȸ�� �����̰� BaseLookupRate�� ���� ȸ�� �����̴�.
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	//ī�޶�� ĳ���Ͱ� ���� ȸ���ϴ� �� ���� ���� �̷��� �������� ������ ȸ�� �� ĳ������ �� ����� ���� �� �� ����.
	//Let that just affect the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; //bOrientRotationToMovement�� �ڵ����� ĳ������ �̵����⿡ ����, ȸ�� ������ ���ش�.
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f); //Ű �Է� �� 540�� ȸ����
	GetCharacterMovement()->JumpZVelocity = 650.f; //�����ϴ� ��
	GetCharacterMovement()->AirControl = 0.2f; //�߷��� ��
	
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

//���� �÷��� �� ������ �Ǵ� �κ�
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
 
//�θ� Ŭ������ �ִ� ������ ���� ������ �����ϱ⿡�� ������ �߻��� �� �����Ƿ� ������� ������ �������ؼ� ���
//���� ���°� �ƴ� ��� bool ������ true�� �־���
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
		//�̰� ���� �̻��ϰ� ��ӹ��� superŬ������ ĳ���Ϳ��� ������ �ٲ� ���̾���
		Super::Jump();

		if (isClimb)
		{
			LaunchVelocity.X = GetActorForwardVector().X * -500.f;
			LaunchVelocity.Y = GetActorForwardVector().Y * -500.f;
			LaunchVelocity.Z = 0.f;

			//space bar �Է� �� ĳ���͸� ���� �ӵ��� �߻��Ŵ
			LaunchCharacter(LaunchVelocity, bXYOverride, bZOverride);

			isClimb = false;

			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
			GetCharacterMovement()->bOrientRotationToMovement = true; 
			ChangeModeToFly();
		}
	} 
}

/// <summary>
/// GetCharacterMovement()�� EMovementMode�� �ȱ� ������� Ȯ�� ���� ���
/// ���� Ÿ���� Ȯ�� ������ bool ������ isClimb�� true���� �ְ�
/// GetCharacterMovement()�� ���Ͱ��� 0���� �ʱ�ȭ �� EMovementMode�� Flying���� ��ȯ���ش�.
/// 
/// </summary>
void ANelia::ChangeModeToFly()
{
	if (EMovementMode::MOVE_Walking)
	{
		isClimb = true;
		GetCharacterMovement()->Velocity = FVector(0, 0, 0);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		
		//bOrientRotationToMovement�� ĳ���� �̵� ������ ���ϵ��� ȸ���Ѵ�.
		//ī�޶� ��� ������ ���ϵ� ĳ���ʹ� �׻� �ڽ��� �����̴� ������ ���ϰ� �ȴ�.
		//false�̴ϱ� �׷� ī�޶�� ���� ������ ���ϰ� �Ǵ� �� �ǹ�???
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}

/// <summary>
/// Character ���� ���� 40 ���� �밢�� ���� �������� 135�� ��ġ�� ���� ���� �����ϰ�
/// �� ����� �밢�� ���� �����Ǿ������� bool ���� ���ο� ���� �������� ���� �ʷϻ��� ���������� ��Ÿ���ְ�
/// ���� ���� ���� �� �浹 �� onClimbLedge���� ���� �밢�� �� ���⿡ �ִ� ���� �浹�� ���� �ִ��� 
/// ���θ� ��Ÿ���� bool ���� ���� -> ChangeModeToFly()�Լ��� �̵����ϰ� �ǰ� �Ȱ� �ִ� ���¶�� 
/// �� �� �� ���¸� Flying ���� ��ȯ �������� �ʾҴµ� iscClimb ������ true�� ��� false�� ��ȯ���ְ�
/// EMovemet ��� ���¸� Walking���� ��ȯ �߰������� ī�޶� ���� ����� ���� ���� ĳ���Ͱ� �̵��� �� �ֵ��� ��
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
		
		//���� ���� �� �ִϸ��̼� ����
		
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (isClimb && !onClimbLedge)
		{

			//UE_LOG(LogTemp, Warning, TEXT("ledge"));

			isClimbUp = true;
			isClimb = false;
			AnimInstance->Montage_Play(ClimbTop_Two, 1.2f);

			//Timer(������)
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

//spacebar�� ������ ���� ȣ��Ǵ� �Լ� bool������ false ���� �־��� 
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

	//���� �ٶ󺸰� �ְ� ��������� ������ ����ǰ� 
	//LookAtYaw = ���� ����� ��ġ�� �ް� ���� ���� �÷��̾��� ȸ�� ������ InterpRotation = �÷��̾��� ȸ�� ����, ������� ��ġ, �����Ӵ� , ���� �ӵ�)�� 
	//�޾� SetActorRotation(InterpRotation)�� ����Ͽ� ������ �Ѵ�. �� ���� ����� ��ġ�� �ް� ����� �ٶ� ���� ���� ȸ�� ���� �޾Ƽ� SetActorRotation(InterpRotation)�� ���� �����Ѵ�.
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
	check(PlayerInputComponent);//�Է¹��� Ű�� Ȯ��

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
	if (CanMove(Value))
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

//��,�� �̵� Value ���� ���� �����ص� 1,-1 ���� �޾� �����δ�.
//����ڷκ��� W,SŰ�� �Է� ������ ȣ��Ǵ� �Լ��̰� ��Ʈ�ѷ��� �ְ� Value�� 0�� �ƴϰ� 
//���� ���� ���� �Ǵ����� ���°� �ƴ� ��� ȸ������ ������ �޾� �̵��Ѵ�.
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

	//�ӵ��� �� ���� �������ְ� ������ �ش� wallUpDown ���� ��ȯ���൵ �ӵ��� ��ȭ���� ���� 
	if (isClimb)
	{
		wallUpDown = Value * 1000.f;
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Z);

		//test->>>> ���� GetCharacterMovement()->Velocity = FVector(0, 0, 1000.f); ������
		if (Value != 0.0f)
		{
			GetCharacterMovement()->Velocity = FVector(0, 0, Value*150.f);
		}
		//������ �ȵǸ� �Ʒ��� 
		//GetCharacterMovement()->Velocity = FVector(0, 0, Value*1000.f);

		//1.0f, -1.f ���� �ִ� test �κ�
		AddMovementInput(Direction, wallUpDown);

		//UE_LOG(LogTemp, Warning, TEXT("updown %f"), wallUpDown);
	}
}
//Log_Type 1
//UE_LOG(LogTemp, Warning, TEXT("updown %f"), wallUpDown);
//UE_LOG(LogTemp, Warning, TEXT("direction %s"), *Direction.ToString());

//��,�� �̵� �� MoveForward�� ���� ����� ����
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

//Ű�� ������ ������ ��Ʈ�ѷ��� 1�ʾȿ� 65�� ȸ�� ���� 
//void ANelia::TurnAtRate(float Rate)
//{
//	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
//}
//
//void ANelia::LookUpAtRate(float Rate)
//{
//	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
//}

//���� ���°� �ƴϰ� Item���� ������ Collision volume �ȿ� ������ �������� �ְ� �������� �ƴ϶��
//������ �������� weapon �������� ��ȯ�Ͽ� Ŭ���� ���� weapon�� �־��ְ� �ش� weapon�� ���� ���� SetActiveOverlappingitem��
//���� �����۵��� ��� ����� ���� ���⸦ �̹� �����ϰ� ������ combo attack���� ���Ǵ� saveattack�� true�� �ϰ� 
//���� ���� �ƴҰ�� Attack()�Լ��� ȣ����
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

//ESC, Q �Է� �� �ش� Pause �޴� â�� �� ���� ���������� ESC �Է� �� �÷��̰� �����Ǳ� ������ �������� Q�� ���� ���� 
void ANelia::ESCDown()
{
	bESCDown = true;

	if (MainPlayerController)
	{
		//MainPlayerController���� �Լ� ���� �޴�â�� �������� ���� ���������� ���� �Լ�
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

//�ǵ帲
void ANelia::BlockEnd()
{
	bAttacking = false;
}


//ü�� ���� �Լ� 
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

//ü�� ���� �Լ�
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

//CombatMontage�� 1.2�� �ӵ��� �ִϸ��̼��� �����ϰ� CombatMontage�� ��Ÿ�� ���� Death �κ����� �̵�
void ANelia::Die()
{
	MainPlayerController = Cast<AMainPlayerController>(GetController());
	if (MovementStatus == EMovementStatus::EMS_Death) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	//MainPlayerController->startplayer ///////////////////// �̰� character spawn �Ϸ��� �ϴ��ǵ� �� startplayer�� ��� �޾ƾ� ���� �𸣰���
	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.2f);
		AnimInstance->Montage_JumpToSection(FName("Death"));	
	}
	SetMovementStatus(EMovementStatus::EMS_Death);
	OnDeath();
}

//������ �ִϸ��̼��� ���߰� ���̷��� ������Ʈ�� �����.
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

//shift Ű�� ������ ���� ȣ��Ǵ� �Լ� 
void ANelia::ShiftKeyDown()
{
	bShiftKeyDown = true;
}

//shift Ű�� ������ ���� �� ȣ��Ǵ� �Լ� 
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

//���� ���� ���°� �ƴϰ� ���� �ʾ��� �� �� ������ �ٶ󺸰� Nelia�� AnimInstance�� ������
//case0���� 2���� ������� �����ϰ� ������ Attack3�� ���� �� �ٽ� Attack1������ ���� ����� ������
//CombatMontage���� ���� �ִϸ��̼Ǹ��� savecombo�� resetcombo�� �������־��� savecombo�� reset combo ���̿� ���� Ű�� ����ؼ� ������ 
//1��, 2��, 3�� �⺻ ���� ������ ���������� �ð��� �ΰ� �⺻ ���� Ű �Է� �� ����ؼ� reset combo���� ���ݸ���� 0���� �ʱ�ȭ�Ǳ� ������
//1�� ���ݸ� �ϵ��� ����
void ANelia::Attack()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Death && EquippedWeapon)
	{
		bAttacking = true;
		SetInterpToEnemy(true);

		//MainPlayerController�� ������ �÷��̾ �Է��� ��ų Ȯ�� �Լ����� ��ȯ�� Ű ���� pressSillNum�� ����
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

			///////////// �÷��̾ �Է��� ��ų Ű�� ���� ��ų�� �����ϴ� �κ� �����ؾ��� 
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
 

//������ ���� �ʰ� ���� �ʾҰ� W,S �Ǵ� A,DŰ�� ������ ���� �� Nelia�� AnimInstance�� �������� RollMontage�� 1.5�� �ӵ��� �����Ѵ�.
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

					// ���⿡ �ڵ带 ġ�� �ȴ�.

				}), 5.f, false);
		}
	}
}

void ANelia::StopRoll()
{
	bRoll = false;
}

//������ ���⿡ SwingSound�� ������ �Ҹ��� ���
void ANelia::PlaySwingSound()
{ 
	if (EquippedWeapon->SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}

//���� �� �� ������ ���ϰ� ��
void ANelia::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

//Nelia ü�� - ���������� 0���� �۰ų� ������ Health -= DamageAmount;�� ���ְ� Die()�Լ� ȣ�� ���� Nelia���� �������� �� �����(DamageCauser)�� Enemy�̸� ��ȿ Ÿ���� false�� ����
//Health - DamageAmount <= 0.f�� �ƴ� ��� ü�¿��� ������ ��ŭ ���ְ� ����
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


//OverlappingActors��� �迭(������� �����Ǵ�)�� ����� �迭�� �ƹ��͵� ���� MainPlayerController�̸� �� ü�¹ٸ� �� ���̵��� �Ѵ�.
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
	//OverlappingActors �迭 ù ��° ��Ҹ� Enemy�� �� ��ȯ�Ͽ� ClosestEnemy�� �ְ� ��� ��ġ���� �ּҰŸ��� ����
	//OverlappingActors ù ��° ��Һ��� ������� AEnemy ���·� ����ȯ �� ���� ����� ���� ���Ͽ� ü�¹ٸ� �� �� �ֵ��� �����ϰ� ���� ����� ���� ����� ������ �����Ѵ�.
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

//���� ���带 �޾ƿ��� ���� �� �̸��� CurrentLevel�� �Է� CurrentLevelName�� �Ű����� LevleName�� �ٸ��� LevleName���� �̵�
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

	////���� ĳ���Ͱ� �ִ� ���� �̸� StreamingLevelPrefix�� �ϴϱ� UEDPIE_0_ElvenRuins �̷��� �� �߰� 
	////�� �̸��� ElvenRuins�� ��
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

// ����ڰ� eŰ�� �Է����� ��� ����Ǵ� �Լ�
void ANelia::Interact()
{	
	/// <summary>
	/// bCanUseDialogue�� ���� �������Ʈ���� npc �ݸ����� �÷��̾ ����Ǿ��� ���� �� �� ��ü���� 
	/// EŰ�� ������ �� ��ȭâ�� �ߴ� ���� �ƴ϶� npc ��ó�� ���� ���� �ö������ �ؾ��ϱ� ������
	/// bool���� bCanUseDialogue�� ���� Ȯ��
	/// </summary>
	if (bCanUseDialogue)
	{
		/// <summary>
		/// MainPlayerController���� �� ������ UserInterface�� �־��ִµ� �������̱� 
		/// ������ ���� Ȯ���ϰ� �Էµ� CurrentState !=3�� �ǹ̴� ��ȭ�ý��� �߿� ��縦 ������ �� �ִ� ��찡 �ִµ�
		/// �ش� �κ��� UserInterface.cpp���� CurrentState 3�� ��� �����ϵ��� �ڵ尡 �����Ǿ� �ִµ� 
		/// ��ȭâ�� ���ִ� ��� eŰ�� �� ���� ������ ������� �� ��ȭ �� npc�� �÷��̾ ��ȭ�� �ְ� 
		/// �ش� npc ��翡 ���� �÷��̾��� ���� ��簡 ������ ��� eŰ�� �Է��ϸ� ���� ��ȭ�� �Ѿ������ ���� ����
		/// </summary>
		if (MainPlayerController->UserInterface != nullptr && (MainPlayerController->UserInterface->CurrentState !=3))
		{
			//UE_LOG(LogTemp, Warning, TEXT("nelia interact in"));
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
