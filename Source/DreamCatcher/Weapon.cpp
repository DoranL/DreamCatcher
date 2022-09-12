// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Nelia.h"
#include "Engine/SkeletalMeshSocket.h"

AWeapon::AWeapon()
{
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());
}

void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	/*if ((WeaponState == EWeaponState::EWS_Pickup) && OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			Main->SetActiveOverlappingItem(this);
		}
	}*/
}
void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	if (OtherActor)
	{
		ANelia* Nelia = Cast<ANelia>(OtherActor);
		if (Nelia)
		{
			//Nelia->SetActiveOverlappingItem(nullptr);
			Equip(Nelia);
		}
	}
}

void AWeapon::Equip(ANelia* Char)
{
	//메쉬를 소켓에 연결
	if (Char)
	{

		//SetInstigator(Char->GetController());

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

			//	Char->SetEquippedWeapon(this);
			//	Char->SetActiveOverlappingItem(nullptr);
			//}
			//if (OnEquipSound) UGameplayStatics::PlaySound2D(this, OnEquipSound);
			//if (!bWeaponParticles)
			//{
			//	IdleParticlesComponent->Deactivate();
			//}
		}

	}
}