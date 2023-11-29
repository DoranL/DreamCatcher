// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "Boss.generated.h"

/**
 * 
 */
UCLASS()
class DREAMCATCHER_API ABoss : public AEnemy
{
	GENERATED_BODY()
	
public:
	ABoss();

	void  Attack() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:
	//virtual void AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	bool bGetup;
	int countHit;
	float timerValue;

protected:
	void GetUpEnd();

public:
	int EnemyAttackCounting;
};
