// Fill out your copyright notice in the Description page of Project Settings.


#include "Collider.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "ColliderMovementComponent.h"
// Sets default values
ACollider::ACollider()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);

	SphereComponent->InitSphereRadius(40.0f);   //충돌 범위
	SphereComponent->SetCollisionProfileName(TEXT("Pawn"));   //충돌 시 이름으로 구분 가능

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshComponentAsset(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));

	if (MeshComponentAsset.Succeeded()) //생성자 도우미 F 객체 찾기 성공
	{
		MeshComponent->SetStaticMesh(MeshComponentAsset.Object); // 해당 개체는 구성 요소 자산과 일치하다?
		MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -40.f)); //반지름 크기만큼 아래에 생성되기 때문에 -40을 해서 구를 가운데에 맞춤
		MeshComponent->SetWorldScale3D(FVector(0.8f, 0.8f, 0.8f));
	}

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f));
	SpringArm->TargetArmLength = 400.f;  //Collider로부터 떨어진 거리
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	OurMovementComponent = CreateDefaultSubobject<UColliderMovementComponent>(TEXT("OurMovementComponent"));
	OurMovementComponent->UpdatedComponent = RootComponent;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}


// Called when the game starts or when spawned
void ACollider::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACollider::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACollider::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACollider::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACollider::MoveRight);
}

void ACollider::MoveForward(float input)
{
	FVector Forward = GetActorForwardVector();

	if (OurMovementComponent)
	{
		OurMovementComponent->AddInputVector(Forward * input);
	}
}

void ACollider::MoveRight(float input)
{
	FVector Right = GetActorRightVector();

	if (OurMovementComponent)
	{
		OurMovementComponent->AddInputVector(Right * input);
	}
}

UPawnMovementComponent* ACollider::GetMovementComponent() const
{
	return OurMovementComponent;
}