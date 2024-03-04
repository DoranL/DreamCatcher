#include "CoreMinimal.h"                   //�𸮾� ������Ʈ�� ������ �� �ִ� �ּ� ��ɸ� ����� ��� ���� ex)Templates , Generic, Containers, Math ����
#include "GameFramework/Character.h"
#include "Nelia.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Normal		 UMETA(DisplayName = "Normal"),
	EMS_Sprinting	 UMETA(DisplayName = "Sprinting"),
	EMS_Death		 UMETA(DisplayName = "Dead"),
	EMS_Block		 UMETA(DisplayName = "Block"),
	EMS_Parry		 UMETA(DisplayName = "Parry"),

	EMS_MAX          UMETA(DisplayName = "Max"),
};

//������
//Blueprint���� 
UENUM(BlueprintType)						
enum class EStaminaStatus : uint8
{
	ESS_Normal				UMETA(DisplayName = "Normal"),
	ESS_BelowMinimum		UMETA(DisplayName = "BelowMinimum"),
	ESS_Exhausted		    UMETA(DisplayName = "Exhausted"),
	ESS_ExhaustedRecovering UMETA(DisplayName = "ExhaustedRecovering"),

	ESS_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class DREAMCATCHER_API ANelia : public ACharacter
{
	GENERATED_BODY()
public:
	// Sets default values for this character's properties
	ANelia();

	UPROPERTY(EditDefaultsOnly, Category = "SaveData")
	TSubclassOf<class AItemStorage> WeaponStorage;

	class AItemStorage* Storage;

	FORCEINLINE void SetHasCombatTarget(bool HasTarget) { bHasCombatTarget = HasTarget; }

	//�Ӽ� â���� ������ �����ϰ� ��������Ϳ��� �б⾲�Ⱑ ��� ������ UParticleSystem Ŭ������ ������ HitParticles ���� - Nelia�� ���� �����ϰ� �� �ݶ��̴��� �ε����� �� ������ ��ƼŬ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* HitParticles;

	//�Ӽ� â���� ������ �����ϰ� ��������Ϳ��� �б⾲�Ⱑ ��� ������ USoundCue Ŭ������ ������ HitSound ���� - Nelia�� ���� �����ϰ� �� �ݶ��̴��� �ε����� �� ������ ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* HitSound;

	//�Ӽ� â���� �������� ������ �Ұ����ϰ� ��������Ϳ��� �б⾲�Ⱑ ������ ������ ������ enum Ŭ������ EMovementStatus�� MovementStatus ���� ���� 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;

	//�Ӽ� â���� �������� ������ �Ұ����ϰ� ��������Ϳ��� �б⾲�Ⱑ ������ ������ ������ enum Ŭ������ EStaminaSatus�� StaminaStatus ���� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminaStatus StaminaStatus;

	//�ζ��� �Լ��� ������ �����ϴ� FORCEINLINE�� ����Ͽ� /////////////// �Լ�ȣ�� �κ��� ������ �����´�>??????//
	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	//�Ӽ� â���� ������ �����ϰ� ��������Ϳ��� �б⾲�Ⱑ ��� ������ float�� ������ StaminDrainRate ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSprintStamina;

	float InterpSpeed;
	bool bInterpToEnemy;
	void SetInterpToEnemy(bool Interp);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* CombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bOverlappingCombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AEnemy* CombatTarget;
		
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Combat")
	FVector CombatTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AEnemy*> Targets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bHasCombatTarget;			  

	int targetIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* MainPlayerController;

	FORCEINLINE void SetCombatTarget(AEnemy* Target) { CombatTarget = Target; }

	FRotator GetLookAtRotationYaw(FVector Target);

	void SetMovementStatus(EMovementStatus Status);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float SprintingSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Running")
	float wallUpDown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Running")
	float wallLeftRight;

	bool bShiftKeyDown;

	void ShiftKeyDown();

	void ShiftKeyUp();

	/**Camera boom positioning the camera behind the player*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAcess = "true"))
	class USpringArmComponent* CameraBoom;

	// Follow Camera 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAcess = "true"))
	class UCameraComponent* FollowCamera;

	void CameraZoom(const float Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float MinZoomLength = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float MaxZoomLength = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float DefaultArmLength = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float ZoomSteps = 30.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	//Player Stats
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int Level;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	bool bCheckLevelUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int Exp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int MaxExp = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int32 PotionCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	FString LevelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	FString WeapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int checkPointCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int32 RandomInt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	bool bTakeDamage;

	UFUNCTION(BlueprintCallable)
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	void IncrementHealth(float Amount);

	UFUNCTION(BlueprintCallable)
	void DecrementHealth(float Amount);

	//leveling system
	UFUNCTION(BlueprintCallable)
	void AddExp();

	UFUNCTION(BlueprintCallable)
	void Die();

	void Escape();

protected:
	virtual void BeginPlay() override;

	virtual void Jump() override;
	virtual void StopJumping() override;
	 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallClimb")
	float TraceDistance;
	 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallClimb")
	float TraceDistanceEnemy;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable)
	void MoveForward(float Value);
	 
	void MoveRight(float Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool isClimb;

	bool onClimbLedge;
	bool isClimbUp;
	void CanClimb();
	void ChangeModeToFly();

	void Turn(float Value);

	void LookUp(float Value);

	bool bMovingForward;
	bool bMovingRight;

	bool CanMove(float Value);

	bool bPickup;
	void PickupPress();

	bool bESCDown;
	void ESCDown();
	void ESCUp();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Parry")
	bool isBlock;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Items)
	class AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
    class AItem* ActiveOverlappingItem;

	FORCEINLINE void SetEquippedWeapon(AWeapon* WeaponToSet) { EquippedWeapon = WeaponToSet; }

	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Anims")
	bool bAttacking;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Anims")
	bool saveAttack;

	UFUNCTION(BlueprintCallable)
	void ResetCombo();

	UFUNCTION(BlueprintCallable)
	void SaveComboAttack();

	int AttackMotionCount = 0;

	UFUNCTION(BlueprintCallable)
	void Attack();

	UFUNCTION(BlueprintCallable)
	void Skill();
	
	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	UFUNCTION(BlueprintCallable)
	void SkillEnd();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* HitMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* SkillMontage;

	UPROPERTY(EditAnywhere)
	class UAnimMontage* RollMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* ClimbTop_Two;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skill")
	bool bcanUseSkill;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Skill")
	int pressSkillNum;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Anims")
	bool bRoll;

	void Rolls(float Value);
	
	UFUNCTION(BlueprintCallable)
	void StopRoll();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bJump;

	class UMainAnimInstance* MainAnimInstance;

	void PlaySwingSound();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	void UpdateCombatTarget();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<AEnemy> EnemyFilter;

	//void SwitchLevel(FName LevelNameSwitch);

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame();

	FRotator NeliaRotation;

	UFUNCTION(BlueprintCallable)
	void OnDeath();

	UFUNCTION(BlueprintCallable, Category = "Inputs")
	void Interact();

	UPROPERTY(BlueprintReadWrite)
	bool bCanUseDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DeathDelay;

	bool bTargeting;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int dialogueCheckNum;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spawn")
	FVector RespawnLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category ="Spawn")
	int32 spawnPointCheckNum;

private:
	UFUNCTION(BlueprintCallable)
	void ReviveEnd();

	UFUNCTION(BlueprintCallable)
	void Respawn();

	UFUNCTION(BlueprintCallable)
	void SprintAttack();

	UFUNCTION()
	virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool bTabKeyDown;

	void Targeting();
	void CancelTargeting();
};