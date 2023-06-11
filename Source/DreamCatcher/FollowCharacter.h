#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FollowCharacter.generated.h"


UENUM(BlueprintType)
enum class EArcherMovementStatus :uint8
{
	EMS_Idle				UMETA(DeplayName = "Idle"),
	EMS_MoveToTarget		UMETA(DeplayName = "MoveToTarget"),
	EMS_Attacking			UMETA(DeplayName = "Attacking"),

	EMS_MAX					UMETA(DeplayName = "DefaultMax")
};

UCLASS()
class DREAMCATCHER_API AFollowCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFollowCharacter();

	//캐릭터가 죽고 나서 계속 공격 당하는 걸 막는 bool 변수
	bool bHasValidTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
		EArcherMovementStatus ArcherMovementStatus;

	FORCEINLINE void SetEnemyMovementStatus(EArcherMovementStatus Status) { ArcherMovementStatus = Status; }
	FORCEINLINE EArcherMovementStatus GetArcherMovementStatus() { return ArcherMovementStatus; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
		class USphereComponent* AgroSphere;

	//combatsphere 내에 플레이어가 들어오면 전투
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
		USphereComponent* CombatSphere;

	//왜 AAIController이지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
		class AAIController* AIController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
		float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
		float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
		float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		class UAnimMontage* AttackMontage;

	FTimerHandle AttackTimer;

	int EnemyAttackCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float AttackMinTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float AttackMaxTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Combat")
		FVector CombatTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
		class UBoxComponent* CombatCollision;

	float InterpSpeed;
	bool bInterpToNelia;
	void SetInterpToNelia(bool Interp);

	FVector NeliaLocation;

	FRotator GetLookAtRotationYaw(FVector Target);

	UPROPERTY(EditAnywhere)
		TSubclassOf<class AActor> Projectile;

	UFUNCTION(BlueprintCallable)
		void SpawnProjectile();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
		virtual void AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		virtual void AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
		virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
		void MoveToTarget(class ANelia* Target);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
		bool bOverlappingCombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
		ANelia* CombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
		bool bAttacking;

	UFUNCTION(BlueprintCallable)
		void  Attack();

	UFUNCTION(BlueprintCallable)
		void AttackEnd();

	UFUNCTION(BlueprintCallable)
		void ActivateCollision();

	UFUNCTION(BlueprintCallable)
		void DeactivateCollision();
};