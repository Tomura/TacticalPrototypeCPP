// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "BRSGameMode_Base.h"
#include "TacticalGameModeBase.generated.h"



UCLASS(minimalapi)
class ATacticalGameModeBase : public ABRSGameMode_Base
{
	GENERATED_BODY()

public:
	ATacticalGameModeBase(const FObjectInitializer& OI);

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	virtual void Killed(class AController* Killer, class AController* Victim, class AActor* KilledActor, const class UDamageType* DamageType);

	virtual float GetModifiedDamage(float Damage, AController* EventInstigator, AActor* DamageCauser, AActor* DamagedActor);

	virtual void OnPlayerSetLoadout(class ATacticalPlayerController* PC) {}


protected:
	UPROPERTY(Category = "Game Mode Settings", BlueprintReadWrite, EditAnywhere)
		uint32 bFriendlyFire : 1;
	UPROPERTY(Category = "Game Mode Settings", BlueprintReadWrite, EditAnywhere)
		uint32 bSelfDamage : 1;

	UPROPERTY(Category = "Game Mode Settings", BlueprintReadWrite, EditAnywhere)
		float DamageModifier;
	UPROPERTY(Category = "Game Mode Settings", BlueprintReadWrite, EditAnywhere)
		float EnemyDamageModifier;
	UPROPERTY(Category = "Game Mode Settings", BlueprintReadWrite, EditAnywhere)
		float FriendlyDamageModifier;


	UFUNCTION(Category = "Game Mode Settings", Exec)
		void SetEnemyDamageModifier(float NewValue) { EnemyDamageModifier = NewValue; }
};
