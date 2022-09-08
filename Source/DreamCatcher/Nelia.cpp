// Fill out your copyright notice in the Description page of Project Settings.


#include "Nelia.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
ANelia::ANelia()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Camera boom(����� �ִ� ��� �÷��̾ ���� ���)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));   
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;							//ī�޶� Neila�� 400������ ��ġ�� �������� �����
	CameraBoom->bUsePawnControlRotation = true;						//��Ʈ�ѷ��� ���� CameraArm ȸ��

	
	GetCapsuleComponent()->SetCapsuleSize(17.f, 64.f);				//Nelia�� ĸ�� ���̿� ������ ����

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));		 
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	//Attach the camera to the end of the boom and let the boom adjust to match
	//the controller orientation
	FollowCamera->bUsePawnControlRotation = false;

	//Set our turn rates for input
	//Ű �Է� �� 1�ʵ��� 65�� ȸ��
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

	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;

	bShiftKeyDown = false;

	//Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;
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

}

// Called to bind functionality to input
void ANelia::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);					//Ű���� �Է� ���� ���޹޴� ���� �Լ�
	check(PlayerInputComponent);											//�Է¹��� Ű�� Ȯ��


	//�Է� ���� Ű�� ���� �ش� �̸��� �´� �Լ� ȣ��? JUMP, �̵�, ĳ���� ȸ���� 
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ANelia::Jump);				
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ANelia::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ANelia::ShiftKeyUp);

	//PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &ANelia::LMBDown);
	//PlayerInputComponent->BindAction("LMB", IE_Released, this, &ANelia::LMBUp);

	PlayerInputComponent->BindAxis("MoveForward", this, &ANelia::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANelia::MoveRight);

	PlayerInputComponent->BindAxis("turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ANelia::TurnAtRate);
	PlayerInputComponent->BindAxis("LokkUpRate", this, &ANelia::LookUpAtRate);
}

void ANelia::MoveForward(float Value)
{
	//bMovingForward = false;
	if ((Controller != nullptr)) //&& (Value != 0.0f) && (!bAttacking) && (MovementStatus != EMovementStatus::EMS_Death))
	{
		//find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		//bMovingForward = true;
	}
}

void ANelia::MoveRight(float Value)
{
	//bMovingRight = false;
	if ((Controller != nullptr) && (Value != 0.0f)) //&& (!bAttacking) && (MovementStatus != EMovementStatus::EMS_Death))
	{
		//find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

		//bMovingRight = true;
	}
}

//Ű�� ������ ������ ��Ʈ�ѷ��� 1�ʾȿ� 65�� ȸ�� ����   ///getworld?
void ANelia::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ANelia::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
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