// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"                   //언리얼 오브젝트가 동작할 수 있는 최소 기능만 선언된 헤더 파일 ex)Templates , Generic, Containers, Math 포함
#include "GameFramework/Character.h"
#include "Nelia.generated.h"

UENUM(BlueprintType)					  //블루프린트에서 Nelia 이동 설정할 때 열거형으로 Normal, Sprinting, Dead 중 하나 선택 가능
enum class EMovementStatus : uint8        //enum클래스 열거형 
{
	EMS_Normal		 UMETA(DisplayName = "Normal"), 
	EMS_Sprinting	 UMETA(DisplayName = "Sprinting"),
	EMS_Death		 UMETA(DisplayName = "Dead"),

	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bHasCombatTarget;

	FORCEINLINE void SetHasCombatTarget(bool HasTarget) { bHasCombatTarget = HasTarget; }


	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Combat")
	FVector CombatTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* MainPlayerController;

	//속성 창에서 편집이 가능하고 블루프린터에서 읽기쓰기가 모두 가능한 UParticleSystem 클래스형 변수인 HitParticles 생성 - Nelia가 적을 공격하고 적 콜라이더와 부딪혔을 때 나오는 파티클
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* HitParticles;

	//속성 창에서 편집이 가능하고 블루프린터에서 읽기쓰기가 모두 가능한 USoundCue 클래스형 변수인 HitSound 생성 - Nelia가 적을 공격하고 적 콜라이더와 부딪혔을 때 나오는 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* HitSound;

	//속성 창에서 보여지나 편집이 불가능하고 블루프린터에서 읽기쓰기가 가능한 위에서 선언한 enum 클래스형 EMovementStatus형 MovementStatus 변수 생성 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;

	//속성 창에서 보여지나 편집이 불가능하고 블루프린터에서 읽기쓰기가 가능한 위에서 선언한 enum 클래스형 EStaminaSatus형 StaminaStatus 변수 생성
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminaStatus StaminaStatus;

	//인라인 함수를 강제로 수행하는 FORCEINLINE을 사용하여 /////////////// 함수호출 부분의 내용을 가져온다>??????//
	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	//속성 창에서 편집이 가능하고 블루프린터에서 읽기쓰기가 모두 가능한 float형 변수인 StaminDrainRate 생성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSprintStamina;

	float InterpSpeed;
	bool bInterpToEnemy;
	void SetInterpToEnemy(bool Interp);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AEnemy* CombatTarget;

	FORCEINLINE void SetCombatTarget(AEnemy* Target) { CombatTarget = Target; }

	FRotator GetLookAtRotationYaw(FVector Target);

	/** Set Movement status and running speed */
	void SetMovementStatus(EMovementStatus Status);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float SprintingSpeed;

	bool bShiftKeyDown;

	/** Pressed down to enable sprinting */
	void ShiftKeyDown();

	/** Released to stop sprinting */
	void ShiftKeyUp();

	/**Camera boom positioning the camera behind the player*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAcess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow Camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAcess = "true"))
	class UCameraComponent* FollowCamera;

	/** Base turn rates to scale turning functions for the camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;


	//Player Stats
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int32 Coins;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	void IncrementHealth(float Amount);

	UFUNCTION(BlueprintCallable)
	void DecrementHealth(float Amount);

	void Die(); 

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Jump() override;
	virtual void StopJumping() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** called for forwards/backwards input*/
	void MoveForward(float Value);

	/** called for side to side input*/
	void MoveRight(float Value);

	/** called for Yaw rotation*/
	void Turn(float Value);

	/** called for Pitch rotation*/
	void LookUp(float Value);

	bool bMovingForward;
	bool bMovingRight;

	bool CanMove(float Value);

	/** Called via input to turn at a given rate
	* @parm Rate This is a normalized rate, i.e 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/** Called via input to look up/down at a given rate
	* @parm Rate This is a normalized rate, i.e 1.0 means 100% of desired look up/down rate
	*/
	void LookUpAtRate(float Rate);

	bool bPickup;
	void PickupPress();
	void PickupReleas();

	bool bESCDown;
	void ESCDown();
	void ESCUp();

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Items)
	class AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
    class AItem* ActiveOverlappingItem;

	//장착 무기를 설정 매개변수로 무기를 가지고 (EquippedWeapon = WeaponToSet)	
	FORCEINLINE void SetEquippedWeapon(AWeapon* WeaponToSet) { EquippedWeapon = WeaponToSet; }

	//FORCEINLINE AWeapon* GetEquippedWeapon() { return EquippedWeapon; }
	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Anims")
	bool bAttacking;

	int AttackMotionCount = 0;

	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* SkillMontage;

	/// ////////////////////
	UPROPERTY(EditAnywhere)
	class UAnimMontage* RollMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Anims")
	bool bRoll;

	void Roll();

	UFUNCTION(BlueprintCallable)
	void StopRoll();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bJump;

	class UMainAnimInstance* MainAnimInstance;

	UFUNCTION(BlueprintCallable)
	void PlaySwingSound();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	void UpdateCombatTarget();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<AEnemy> EnemyFilter;

	void SwitchLevel(FName LevelName);

	

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame(bool SetPosition);

	void LoadGameNoSwitch();

	FRotator NeliaRotation;
};