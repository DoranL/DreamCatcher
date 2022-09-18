// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Nelia.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroShere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(600.f);
	//
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(75.f);

	bOverlappingCombatSphere = false;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	//���Ͱ� �÷��̾ ���� �� ī�޶� �������� �� ���� �ڵ�(Mesh(�� �׵θ�), capsule(���� ������ ĸ��)�� ī�޶� �þ߿� �ְԵǸ�
	//�Ʒ� �ڵ尡 ���� �� ĳ���͸� �����ֱ� ���� ������ ������� �Ǵµ� �� �ɸ� �� ó�� �������� �� ���� �����ϵ��� �ڵ� �ۼ�
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor) //&& Alive())
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			MoveToTarget(Nelia);
		}
	}
}
void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
			if (AIController)
			{
				AIController->StopMovement();
			}
		}
	}
	/*if (OtherActor)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		{
			if (Nelia)
			{
				bHasValidTarget = false;
				if (Nelia->CombatTarget == this)
				{
					Nelia->SetCombatTarget(nullptr);
				}
				Nelia->SetHasCombatTarget(false);

				Nelia->UpdateCombatTarget();

				SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
				if (AIController)
				{
					AIController->StopMovement();
				}
			}
		}
	}*/
}
void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			CombatTarget = Nelia;
			bOverlappingCombatSphere = true;
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
		}
	}
	/*if (OtherActor && Alive())
	{
		ANelia* Nelia = Cast<AMain>(OtherActor);
		{
			if (Nelia)
			{
				bHasValidTarget = true;
				Nelia->SetCombatTarget(this);
				Nelia->SetHasCombatTarget(true);

				Nelia->UpdateCombatTarget();

				if (Nelia->MainPlayerController)
				{
					Nelia->MainPlayerController->DisplayEnemyHealthBar();
				}
				CombatTarget = Nelia;
				bOverlappingCombatSphere = true;

				float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
				GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
			}
		}
	}*/
}
void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			bOverlappingCombatSphere = false;
			if (EnemyMovementStatus != EEnemyMovementStatus::EMS_Attacking)
			{
				MoveToTarget(Nelia);
				CombatTarget = nullptr;
			}
		}
	}
	/*if (OtherActor && OtherComp)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		{
			if (Nelia)
			{
				bOverlappingCombatSphere = false;
				MoveToTarget(Nelia);
				CombatTarget = nullptr;

				if (Nelia->CombatTarget == this)
				{
					Nelia->SetCombatTarget(nullptr);
					Nelia->bHasCombatTarget = false;
					Nelia->UpdateCombatTarget();
				}

				if (Nelia->MainPlayerController)
				{
					USkeletalMeshComponent* NeliaMesh = Cast<USkeletalMeshComponent>(OtherComp);
					if (NeliaMesh) Nelia->MainPlayerController->RemoveEnemyHealthBar();
				}

				GetWorldTimerManager().ClearTimer(AttackTimer);
			}
		}
	}*/
}

//MoveToTarget�� �÷��̾� �ݶ��̴��� enemy �ݶ��̴��� ��ġ�� �Ǹ� ���� �÷��̾� ������ �̵��ϵ��� �Ǿ� �ִ� �Լ��ε�?
void AEnemy::MoveToTarget(ANelia* Target)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if (AIController)
	{
		FAIMoveRequest MoveRequest;
		//Ÿ���� ��ǥ ���ͷ� ����
		MoveRequest.SetGoalActor(Target);
		//�� �� ���� ���� �� Ÿ������ �ν�
		MoveRequest.SetAcceptanceRadius(10.0f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);

		/*auto PathPoints = NavPath->GetPathPoints();
		for (auto Point : PathPoints)
		{
			FVector Location = Point.Location;

			UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 8, FLinearColor::Red, 10.f, 1.5f);
		}*/
	}
}