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

	//ĳ���Ͱ� �װ� ���� ��� ���� ���ϴ� �� ���� bool ����
	bool bHasValidTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EArcherMovementStatus ArcherMovementStatus;

	FORCEINLINE void SetEnemyMovementStatus(EArcherMovementStatus Status) { ArcherMovementStatus = Status; }
	FORCEINLINE EArcherMovementStatus GetArcherMovementStatus() { return ArcherMovementStatus; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class USphereComponent* AgroSphere;

	//combatsphere ���� �÷��̾ ������ ����
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	USphereComponent* CombatSphere;

	//�� AAIController����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AAIController* AIController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float Damage;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UAnimMontage* AttackMontage;

	int EnemyAttackCount = 0;*/

	FTimerHandle AttackTimer;

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
	bool bInterpToEnemy;

	void SetInterpToEnemy(bool Interp);

	FVector EnemyLocation;

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
	class AEnemy* CombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bAttacking;


	//2923-07-18 follow character attack ��Ÿ�� ���� SpawnProjectile�� ���� �ð� �ʿ� �� ���� ȣ�������ν� �����ؼ� attack�� �ʿ���ٰ� �Ǵ�
	/*UFUNCTION(BlueprintCallable)
	void  Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();*/

	UFUNCTION(BlueprintCallable)
	void ActivateCollision();

	UFUNCTION(BlueprintCallable)
	void DeactivateCollision();
};