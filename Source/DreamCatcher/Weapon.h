// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"s
#include "Weapon.generated.h"

/**
 * 
 */
UCLASS()
class DREAMCATCHER_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
	AWeapon();

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
	bool bWeaponParticles;*/

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SkeletalMesh")
	class USkeletalMeshComponent* SkeletalMesh;

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)override;

	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	void Equip(class ANelia* Char); 
};
