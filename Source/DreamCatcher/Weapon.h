// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
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



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
	bool bWeaponParticles;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SkeletalMesh")
	class USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item | Combat")
	class UBoxComponent* CombatCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Combat")
	float Damage;

protected:
	virtual void BeginPlay() override;

public:

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)override;

	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	void Equip(class ANelia* Char); 

	UFUNCTION()
	void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//UFUNCTION(BlueprintCallable)
	//void ActivateCollision();

	//UFUNCTION(BlueprintCallable)
	//void DeactivateCollision();

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	//TSubclassOf<UDamageType> DamageTypeClass;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	//AController* WeaponInstigator;

	//FORCEINLINE void SetInstigator(AController* Inst) { WeaponInstigator = Inst; }

};
