// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "Net/UnrealNetwork.h"
#include "BRSActorHealthComponent.h"
#include "TacticalPlayerController.h"
#include "TacticalGameMode.h"


// Sets default values for this component's properties
UBRSActorHealthComponent::UBRSActorHealthComponent()
{
	bAutoRegister = true;
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);
	bAutoActivate = true;
	// ...
	Health = 100.f;
	MaxHealth = 100.f;
	bDied = false;
}

void UBRSActorHealthComponent::InitializeComponent()
{
	Super::InitializeComponent();
	// Register Damage Events from owner
	Health = MaxHealth;
	if (GetOwner() && GetOwner()->Role == ROLE_Authority)
	{
		GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UBRSActorHealthComponent::OnTakeDamage);
	}
}

void UBRSActorHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UBRSActorHealthComponent, Health);
	DOREPLIFETIME(UBRSActorHealthComponent, MaxHealth);
	DOREPLIFETIME(UBRSActorHealthComponent, LastHitInfo)
	DOREPLIFETIME(UBRSActorHealthComponent, bDied);
}

// Called when the game starts
void UBRSActorHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UBRSActorHealthComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

void UBRSActorHealthComponent::OnTakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (!GetOwner())
		return;
	// don't do anything if Pawn is already dead
	if (Health <= 0 || bDied){ return; }

	float ActualDamage = Damage;

	//// Notify Instigator Controller about Hit
	//ATacticalPlayerController* InstigatorPC = Cast<ATacticalPlayerController>(InstigatedBy);
	//if (InstigatorPC)
	//{
	//	InstigatorPC->ClientNotifyWeaponHit();
	//}

	// Apply Damage to Health
	Health = FMath::Max(Health - ActualDamage, 0.f);

	const bool bKilled = (!bDied && Health <= 0);


	if (GetOwnerRole() == ROLE_Authority)
	{
		ReplicateHit(Damage, DamageType, InstigatedBy ? InstigatedBy->GetPawn() : nullptr, DamageCauser, bKilled);
	}
	// Check whether Pawn dies or just gets damaged
	if (bKilled)
	{
		UE_LOG(LogTemp, Log, TEXT("Die!!!"));
		Die(ActualDamage, DamageType, InstigatedBy, DamageCauser);
	}
	else
	{
		OnDamage.Broadcast(Damage, DamageType, InstigatedBy ? InstigatedBy->GetPawn() : nullptr, DamageCauser);
	}
}


bool UBRSActorHealthComponent::CanDie(float KillingDamage, const class UDamageType* DamageType, AController* Killer, AActor* DamageCauser) const
{
	AGameModeBase* const GMB = GetWorld()->GetAuthGameMode();
	if (!GMB)
	{
		return false;
	}
	AGameMode* const GM = Cast<AGameMode>(GMB);
	if (!GetOwner()
		|| bDied											// already dead
		|| GetOwner()->IsPendingKill()						// already destroyed
		|| GetOwnerRole() != ROLE_Authority					// not authority
		|| GM == nullptr
		|| GM->HasMatchEnded())	// Has Match Ended?
	{
		return false;
	}
	return true;
}


bool UBRSActorHealthComponent::IsAlive() const
{
	return !bDied;
}

float UBRSActorHealthComponent::GetCurrentHealth() const
{
	return Health;
}

float UBRSActorHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

float UBRSActorHealthComponent::GetRelativeHealth() const
{
	return (Health / MaxHealth);
}

bool UBRSActorHealthComponent::Die(float KillingDamage, const class UDamageType* DamageType, AController* Killer, AActor* DamageCauser)
{
	if (!GetOwner() || !CanDie(KillingDamage, DamageType, Killer, DamageCauser))
	{
		return false;
	}

	Health = 0.f;
	bDied = true;

	//TODO if environmental damage refer to previous damage causer/killer (knocked into trap/off cliff)

	// Get Pawns controller
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	AController* const KilledPlayer = (OwnerPawn) ? OwnerPawn->Controller : nullptr;

	//Notify Game Mode about Kill
	if (GetOwnerRole() == ROLE_Authority && GetWorld()->GetAuthGameMode<ATacticalGameMode>())
	{
		GetWorld()->GetAuthGameMode<ATacticalGameMode>()->Killed(Killer, KilledPlayer, GetOwner(), DamageType);
	}

	if (AActor* DefaultOwner = GetOwner()->GetClass()->GetDefaultObject<AActor>())
	{
		GetOwner()->NetUpdateFrequency = DefaultOwner->NetUpdateFrequency;
	}

	OnDeath.Broadcast(KillingDamage, DamageType, Killer ? Killer->GetPawn() : nullptr, DamageCauser);

	return true;
}


void UBRSActorHealthComponent::ReplicateHit(float Damage, const class UDamageType* DamageType, class APawn* PawnInstigator, class AActor* DamageCauser, bool bKilled)
{
	const float TimeoutTime = GetWorld()->GetTimeSeconds() + 0.5f;

//	FDamageEvent const& LastDamageEvent = LastHitInfo.GetDamageEvent();

	if ((PawnInstigator == LastHitInfo.PawnInstigator.Get()) 
		/*&& (LastDamageEvent.DamageTypeClass == LastHitInfo.DamageTypeClass) */
		&& (LastHitTimeoutTime == TimeoutTime))
	{
		// same frame damage
		if (bKilled && LastHitInfo.bKilled)
		{
			// Redundant death take hit, just ignore it
			return;
		}

		// otherwise, accumulate damage done this frame
		Damage += LastHitInfo.ActualDamage;
	}

	LastHitInfo.ActualDamage = Damage;
	LastHitInfo.PawnInstigator = PawnInstigator;
	LastHitInfo.DamageCauser = DamageCauser;
	//LastHitInfo.SetDamageEvent(DamageEvent);
	LastHitInfo.DamageTypeClass = DamageType->GetClass();
	LastHitInfo.bKilled = bKilled;
	LastHitInfo.EnsureReplication();

	LastHitTimeoutTime = TimeoutTime;
}

void UBRSActorHealthComponent::OnRep_LastHit()
{
	if (LastHitInfo.bKilled)
	{
		OnDeath.Broadcast(LastHitInfo.ActualDamage, LastHitInfo.DamageTypeClass->GetDefaultObject<UDamageType>(), LastHitInfo.PawnInstigator.Get(), LastHitInfo.DamageCauser.Get());
	}
	else
	{
		OnDamage.Broadcast(LastHitInfo.ActualDamage, LastHitInfo.DamageTypeClass->GetDefaultObject<UDamageType>(), LastHitInfo.PawnInstigator.Get(), LastHitInfo.DamageCauser.Get());
	}
}
