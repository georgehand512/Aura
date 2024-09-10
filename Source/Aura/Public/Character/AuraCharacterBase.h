// Copyright GH Indie Developments

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "AuraCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class UAnimMontage;
class UNiagaraSystem;
class UDebuffNiagaraComponent;
class UPassiveNiagaraComponent;

UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AAuraCharacterBase();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	//////////////////////////////////////////////////////////
	/** Combat Interface */

	// Hit React Montage
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;

	// Handling Death of Characters - override function from combat interface
	virtual void Die(const FVector& DeathImpulse) override;

	virtual FOnDeathSignature& GetOnDeathDelegate() override;	
	
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag) override;

	// Get alive players within radius
	bool IsDead_Implementation() const override;
	AActor* GetAvatar_Implementation() override;

	// Tagged Montage
	TArray<FTaggedMontage> GetAttackMontages_Implementation() override;
	
	// Impact Effects
	virtual UNiagaraSystem* GetBloodEffect_Implementation() override;

	virtual FTaggedMontage getTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag) override;

	// get spawned minion count prevent too many being spawned simultaniously
	virtual int32 getMinionCount_Implementation() override;

	// Increment mMinion count
	virtual void UpdateMinionCount_Implementation(int32 Amount) override;

	virtual ECharacterClass GetCharacterClass_Implementation() override;

	virtual FOnASCRegistered& GetOnASCRegisteredDelegate() override;

	virtual USkeletalMeshComponent* GetWeapon_Implementation() override;
	virtual void SetIsBeingShocked_Implementation(bool bInShock) override;
	virtual bool IsBeingShocked_Implementation() const override;

	virtual FOnDamageSignature& GetOnDamageSignature() override;
	
	/** End - Combat Interface */
	////////////////////////////////////////////////////////////

	FOnASCRegistered OnAscRegistered;
	FOnDeathSignature OnDeathDelegate;
	FOnDamageSignature OnDamageDelegate;

	// Handling Death of Characters (L_138) - comms from server to client etc multicast RPC.
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath(const FVector& DeathImpulse);

	// Tagged Montage
	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FTaggedMontage> AttackMontages;

	UPROPERTY(ReplicatedUsing=OnRep_Stunned, BlueprintReadOnly)
	bool bIsStunned = false;

	UPROPERTY(ReplicatedUsing = OnRep_Burned, BlueprintReadOnly)
	bool bIsBurned = false;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsBeingShocked = false;

	UFUNCTION()
	virtual void OnRep_Stunned();

	UFUNCTION()
	virtual void OnRep_Burned();

	void SetCharacterClass(ECharacterClass InClass) { CharacterClass = InClass; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName; // socketname for location to fire projectiles from

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName LeftHandSocketName; // socketname for location to fire projectiles from

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName RightHandSocketName; // socketname for location to fire projectiles from

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName TailSocketName; // socketname for location to fire projectiles from

	// Get alive players within radius
	UPROPERTY(BlueprintReadOnly)
	bool bDead = false;

	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 600.f;

	UPROPERTY();
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY();
	TObjectPtr<UAttributeSet> AttributeSet;

	virtual void InitAbilityActorInfo();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float level) const;
	virtual void InitializeDefaultAttributes() const;


	void AddCharacterAbilities();

	// Dissolve death effects
	void Dissolve();

	// Blueprint function as easier to implement in blueprints
	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	// Blueprint function as easier to implement in blueprints
	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	// Dissolve effects
	UPROPERTY(Editanywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	// Dissolve effects
	UPROPERTY(Editanywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;

	// Impact Effects
	UPROPERTY(Editanywhere, BlueprintReadOnly, Category = "Combat")
	UNiagaraSystem* BloodEffect;

	// Dead sound
	UPROPERTY(Editanywhere, BlueprintReadOnly, Category = "Combat")
	USoundBase* DeathSound;

	/** Minions */

	int32 MinionCount = 0;

	// Adding CharacterClassInfo to Enemy
	// moved from Enemy.h -> XP reward For enemies
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults");
	ECharacterClass CharacterClass = ECharacterClass::Warrior;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDebuffNiagaraComponent> BurnDebuffComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDebuffNiagaraComponent> StunDebuffComponent;

private:

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	// Passively listen for events
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;

	// Hit React Montage
	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> HaloOfProtectionNiagaraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> LifeSiphonNiagaraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> ManaSiphonNiagaraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> EffectAttachComponent;
};

