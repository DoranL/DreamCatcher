// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Nelia.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
//#include "Particles/ParticleSystemComponent.h"

AWeapon::AWeapon()
{
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());

	//bWeaponParticles = false;
}

void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
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
			//�̰� ������ ����� �÷��̾� �ݶ��̴��� ��ġ�� ������ ���⸦ �����ϰԵ�
			Nelia->SetActiveOverlappingItem(nullptr);
		}
	}
}

void AWeapon::Equip(ANelia* Char)
{
	//�޽��� ���Ͽ� ����
	if (Char)
	{

		//SetInstigator(Char->GetController());

		//�̰� ���Ⱑ ī�޶� ������ Ȯ������ �ʰ� �����Ѵٴ� �����ΰ���?
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

		//�浹�� �������� ó��?
		SkeletalMesh->SetSimulatePhysics(false);

		//ĳ���� skeletalmesh ������ �տ� ������ ���� �̸����� �ҷ���
		const USkeletalMeshSocket* RightHandSocket = Char->GetMesh()->GetSocketByName("RightHandSocket");
		//������ ������ ���� �ƴϱ� ������ ĳ���Ϳ� ����
		if (RightHandSocket)
		{
			RightHandSocket->AttachActor(this, Char->GetMesh());
			//	bRotate = false;

			Char->SetEquippedWeapon(this);
			//	Char->SetActiveOverlappingItem(nullptr);
			//}
			//if (OnEquipSound) UGameplayStatics::PlaySound2D(this, OnEquipSound);
			/*if (!bWeaponParticles)
			{
				IdleParticlesComponent->Deactivate();
			}*/
		}

	}
}