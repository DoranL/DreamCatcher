// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Nelia.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Enemy.h"

AWeapon::AWeapon()
{
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetRootComponent());

	bWeaponParticles = false;

	Damage = 25.f;
}


void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapEnd);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

}

void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (OtherActor)//(WeaponState == EWeaponState::EWS_Pickup) && OtherActor)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			Nelia->SetActiveOverlappingItem(this);
		}
	}
}
void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	if (OtherActor)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			//이게 없으면 무기랑 플레이어 콜라이더가 겹치면 무조건 무기를 장착하게됨
			Nelia->SetActiveOverlappingItem(nullptr);
		}
	}
}

void AWeapon::Equip(ANelia* Char)
{
	//메쉬를 소켓에 연결
	if (Char)
	{

		SetInstigator(Char->GetController());

		//이게 무기가 카메라를 가려도 확대하지 않고 무시한다는 내용인가요?
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

		//충돌과 물리학을 처리?
		SkeletalMesh->SetSimulatePhysics(false);

		//캐릭터 skeletalmesh 오른쪽 손에 생성한 소켓 이름으로 불러옴
		const USkeletalMeshSocket* RightHandSocket = Char->GetMesh()->GetSocketByName("RightHandSocket");
		//오른쪽 소켓이 널이 아니기 때문에 캐릭터에 연결
		if (RightHandSocket)
		{
			RightHandSocket->AttachActor(this, Char->GetMesh());
			//	bRotate = false;

			Char->SetEquippedWeapon(this);
			Char->SetActiveOverlappingItem(nullptr);
			
			//if (OnEquipSound) UGameplayStatics::PlaySound2D(this, OnEquipSound);
			if (!bWeaponParticles)
			{
				IdleParticlesComponent->Deactivate();
			}
		}

	}
}

void AWeapon::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			UE_LOG(LogTemp, Warning, TEXT("FIRST"));
			const USkeletalMeshSocket* WeaponSocket = SkeletalMesh->GetSocketByName("WeaponSocket");
			if (WeaponSocket)
			{

				FVector SocketLocation = WeaponSocket->GetSocketLocation(SkeletalMesh);
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticles, SocketLocation, FRotator(0.f), false);
			}
			/*if (Enemy->HitParticles)
			{
				
				const USkeletalMeshSocket* WeaponSocket = SkeletalMesh->GetSocketByName("WeaponSocket");
				if (WeaponSocket && )
				{
					
					FVector SocketLocation = WeaponSocket->GetSocketLocation(SkeletalMesh);
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticles, SocketLocation, FRotator(0.f), false);
				}
			}*/
			if (Enemy->HitSound)
			{
				UGameplayStatics::PlaySound2D(this, Enemy->HitSound);
			}
			if (DamageTypeClass)
			{
				UGameplayStatics::ApplyDamage(Enemy, Damage, WeaponInstigator, this, DamageTypeClass);
			}
		}
	}
}
void AWeapon::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AWeapon::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AWeapon::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}