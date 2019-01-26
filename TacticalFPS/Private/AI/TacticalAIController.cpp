// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"

#include "NavigationSystem.h"
	
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Team.h"

#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Team.h"

#include "TacticalAIController.h"
#include "TacticalCharacter.h"
#include "TacticalCharacterMovement.h"
#include "TacticalWeapon.h"
#include "TacticalPathFollowingComponent.h"
#include "TacticalProjectile.h"
#include "TacticalPlayerController.h"
#include "TacticalCoverMarker.h"
#include "TacticalCoverComponent.h"

#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

#include "TacticalAIModifierVolume.h"

#include "TacticalWeaponStateFiring.h"


ATacticalAIController::ATacticalAIController(const FObjectInitializer& OI)
	: Super(OI.SetDefaultSubobjectClass<UTacticalPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
	TeamID = FGenericTeamId(TEAM_Enemies);

	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception Component"));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("Damage Config"));
	TeamConfig = CreateDefaultSubobject<UAISenseConfig_Team>(TEXT("Team Config"));

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->ConfigureSense(*HearingConfig);
	AIPerceptionComp->ConfigureSense(*DamageConfig);
	AIPerceptionComp->ConfigureSense(*TeamConfig);

	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ATacticalAIController::OnSenseUpdate);

	bIsIdle = true;
	bCanCover = false;
	BehaviorTreeAsset = nullptr;
	BlackBoardAsset = nullptr;

	HearingNavQueryFilter = nullptr;

	VisionAngleMargin = 5; // deg

	VisionAngleOverDistance = FRuntimeFloatCurve();
	VisionAngleOverDistance.GetRichCurve()->AddKey(0.f, 90.f);
	VisionAngleOverDistance.GetRichCurve()->AddKey(500.f, 90.f);
	VisionAngleOverDistance.GetRichCurve()->AddKey(1500.f, 10.f);
	VisionAngleOverDistance.GetRichCurve()->AddKey(5000.f, 10.f);


	VisionStrengthOverDistance = FRuntimeFloatCurve();
	VisionStrengthOverDistance.GetRichCurve()->AddKey(0.f, 1.f);
	VisionStrengthOverDistance.GetRichCurve()->AddKey(500.f, 1.f);
	VisionStrengthOverDistance.GetRichCurve()->AddKey(1500.f, 75.f);
	VisionStrengthOverDistance.GetRichCurve()->AddKey(3000.f, 0.25f);
	VisionStrengthOverDistance.GetRichCurve()->AddKey(5000.f, 0.25f);
}

void ATacticalAIController::BeginPlay()
{
	Super::BeginPlay();

	SetGenericTeamId(TeamID);
}

void ATacticalAIController::Possess(APawn* PossessedPawn)
{
	Super::Possess(PossessedPawn);
	
	// == Setup Perception
	// setup sight
	SightConfig->SightRadius = 3000.f;
	SightConfig->LoseSightRadius = 3500.f;
	SightConfig->PeripheralVisionAngleDegrees = 90.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
	
	AIPerceptionComp->ConfigureSense(*SightConfig);

	// setup hearing
	HearingConfig->HearingRange = 2000.f;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = false;
	AIPerceptionComp->ConfigureSense(*HearingConfig);

	// setup damage
	AIPerceptionComp->ConfigureSense(*DamageConfig);

	// setup team
	AIPerceptionComp->ConfigureSense(*TeamConfig);

	// Register Pawn as Stimulus source
	UAIPerceptionSystem::RegisterPerceptionStimuliSource(GetWorld(), DamageConfig->GetSenseImplementation(), PossessedPawn);
	UAIPerceptionSystem::RegisterPerceptionStimuliSource(GetWorld(), TeamConfig->GetSenseImplementation(), PossessedPawn);

	// == Blackboard and Behavior Tree
	// Use Blackboard if override exists
	if (BlackBoardAsset)
	{
		UseBlackboard(BlackBoardAsset, Blackboard);
	}

	// Run Behavior Tree
	bool bRunningBehaviorTree = false;

	// Characters Placed in World can have specific BT overrides
	ATacticalCharacter* myChar = PossessedPawn ? Cast<ATacticalCharacter>(PossessedPawn) : nullptr;
	if (myChar && myChar->GetBehaviorTreeAssetOverride())
	{
		bRunningBehaviorTree = RunBehaviorTree(myChar->GetBehaviorTreeAssetOverride());
	} 
	
	if (!bRunningBehaviorTree && BehaviorTreeAsset)
	{
		bRunningBehaviorTree = RunBehaviorTree(BehaviorTreeAsset);

	}
	// Init Blackboard Keys (for example if our Blackboard needs information about the Default Behavior)
	if (Blackboard && Blackboard->IsValidLowLevel())
	{
		InitializeBlackboardKeys();
	}

}



void ATacticalAIController::UnPossess()
{
	Super::UnPossess();

	// We don't need this anymore. // in case of squad AI maybe 
	SetLifeSpan(0.1f);

}

void ATacticalAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TArray<AActor*> Enemies;

	float MinAlert = MinAlertness;
	float AlertRate = AlertRateScale;

	if (GetPawn() != nullptr)
	{
		ATacticalAIModifierVolume* BestAIMod = nullptr;
		float BestDistSq = -1.f;
		float BestExtSq = -1.f;
		TArray<AActor*> OverlapVol;
		GetPawn()->GetOverlappingActors(OverlapVol, ATacticalAIModifierVolume::StaticClass());

		for (AActor* TestVol : OverlapVol)
		{
			ATacticalAIModifierVolume* TestAIMod = Cast<ATacticalAIModifierVolume>(TestVol);
			if (TestAIMod != nullptr)
			{
				const float TestDistSq = FVector::DistSquared(GetPawn()->GetActorLocation(), TestAIMod->GetActorLocation());
				const float TestExtSq = TestAIMod->GetBrushComponent()->Bounds.BoxExtent.SizeSquared();

				if (BestAIMod == nullptr || TestExtSq < BestExtSq || TestDistSq < BestDistSq)
				{
					BestAIMod = TestAIMod;
					BestDistSq = TestDistSq;
					BestExtSq = TestExtSq;
				}
			}
		}
		if (BestAIMod)
		{
			MinAlert = BestAIMod->GetMinAwareness();
			AlertRate = AlertRateScale * BestAIMod->GetAwarenessIncrementMultiplier();
		}

		if (AIPerceptionComp)
		{
			AIPerceptionComp->GetCurrentlyPerceivedActors(SightConfig->GetSenseImplementation(), Enemies);
			for (int32 i = Enemies.Num() - 1; i >= 0; --i)
			{
				if (Enemies.IsValidIndex(i))
				{
					ATacticalCharacter* EnemyChar = Cast<ATacticalCharacter>(Enemies[i]);
					if (!EnemyChar || GetTeamAttitudeTowards(*Enemies[i]) != ETeamAttitude::Hostile || !EnemyChar->IsAlive())
					{
						Enemies.RemoveAt(i);
					}
				}
			}
		}


		for (class AActor* EnemyActor : Enemies)
		{
			float currentAlert = 0.f;
			float* currentAlertVal = Alertness.Find(EnemyActor);
			if (currentAlertVal != nullptr)
			{
				currentAlert = *currentAlertVal;
			}

			FActorPerceptionBlueprintInfo Info;
			PerceptionComponent->GetActorsPerception(EnemyActor, Info);
			float SenseStrength = 0.f;
			for (const FAIStimulus& TestStimulis : Info.LastSensedStimuli)
			{
				if (TestStimulis.Type != PerceptionComponent->GetDominantSenseID())
					continue;
				SenseStrength = TestStimulis.Strength;
			}

			// scale strength depending on distance and angle
			const FVector myLoc = GetPawn()->GetActorLocation();
			const FVector otherLoc = EnemyActor->GetActorLocation();

			const float Dist = FVector::Distance(myLoc, otherLoc); // alternatively

			SenseStrength *= VisionStrengthOverDistance.GetRichCurve()->Eval(Dist);

			const float Angle = FMath::RadiansToDegrees((otherLoc - myLoc).CosineAngle2D(GetPawn()->GetViewRotation().Vector()));

			const float AngleLimit = VisionAngleOverDistance.GetRichCurve()->Eval(Dist);
			const float AngleDelta = FMath::Max(Angle - AngleLimit, 0.f);
			if (AngleDelta > 0)
			{
				if (AngleDelta > VisionAngleMargin)
				{
					SenseStrength = 0.f;
					continue;
				}
				else
				{
					SenseStrength *= FMath::Lerp(0.f, 1.f, AngleDelta / VisionAngleMargin);
				}
			}

			if (SenseStrength > 0.f)
			{
				Alertness.Add(EnemyActor, FMath::Min(currentAlert + (DeltaTime*(AlertRate*SenseStrength + AlertRegenRate)), MaxAlert));
			}
		}

		Alertness.GenerateKeyArray(Enemies);
		for (class AActor* EnemyActor : Enemies)
		{
			float* currentAlertVal = Alertness.Find(EnemyActor);
			if (currentAlertVal != nullptr)
			{
				float currentAlert = *currentAlertVal;
				Alertness.Add(EnemyActor, FMath::Max(currentAlert - (DeltaTime*AlertRegenRate), MinAlert));
			}
		}

		OverallAlertness = 0.f;
		if (Alertness.Num() > 0)
		{
			float ResultAlert = 0.f;
			TArray<float> ValAlert;
			Alertness.GenerateValueArray(ValAlert);
			for (float& TestVal : ValAlert)
			{
				OverallAlertness = FMath::Max(TestVal, ResultAlert);
			}
		}
		OverallAlertness = FMath::Max(OverallAlertness, MinAlert); // overall alertness level should not be smaller than the minimal alertness
	}
}

float ATacticalAIController::GetAwareness() const
{

	return OverallAlertness;
}


void ATacticalAIController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
	if (PlayerState)
	{
		IGenericTeamAgentInterface* TeamPlayerState = Cast<IGenericTeamAgentInterface>(PlayerState);
		if (TeamPlayerState)
		{
			return TeamPlayerState->SetGenericTeamId(NewTeamID);
		}
	}
}

FGenericTeamId ATacticalAIController::GetGenericTeamId() const
{
	if (PlayerState)
	{
		IGenericTeamAgentInterface* TeamPlayerState = Cast<IGenericTeamAgentInterface>(PlayerState);
		if (TeamPlayerState)
		{
			return TeamPlayerState->GetGenericTeamId();
		}
	}
	return TeamID;
}

ETeamAttitude::Type ATacticalAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (PlayerState)
	{
		IGenericTeamAgentInterface* TeamPlayerState = Cast<IGenericTeamAgentInterface>(PlayerState);
		if (TeamPlayerState)
		{
			return TeamPlayerState->GetTeamAttitudeTowards(Other);
		}
	}

	if (const IGenericTeamAgentInterface* OtherTeamInterface = Cast<IGenericTeamAgentInterface>(&Other))
	{
		const FGenericTeamId OtherTeam = OtherTeamInterface->GetGenericTeamId();
		const bool bSameTeam = (OtherTeam == GetGenericTeamId());
		if (bSameTeam)
		{
			return ETeamAttitude::Friendly;
		}
		else if (OtherTeam == TEAM_Civilians)
		{
			return ETeamAttitude::Neutral;
		}
		else
		{
			return ETeamAttitude::Hostile;
		}
	}

	return ETeamAttitude::Friendly;

	//const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);
	//return OtherTeamAgent ? FGenericTeamId::GetAttitude(GetGenericTeamId(), OtherTeamAgent->GetGenericTeamId())
	//	: ETeamAttitude::Neutral;
}

void ATacticalAIController::OnPawnDied_Implementation(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser)
{

}

void ATacticalAIController::OnPawnTakeDamage_Implementation(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser)
{

}



void ATacticalAIController::GetEnemiesInSight(TArray<AActor*>& OutActors) const
{
	if (AIPerceptionComp)
	{
		AIPerceptionComp->GetCurrentlyPerceivedActors(SightConfig->GetSenseImplementation(), OutActors);
		for (int32 i = OutActors.Num() - 1; i >= 0; --i)
		{
			if (OutActors.IsValidIndex(i))
			{
				ATacticalCharacter* EnemyChar = Cast<ATacticalCharacter>(OutActors[i]);
				if (!EnemyChar || GetTeamAttitudeTowards(*OutActors[i]) != ETeamAttitude::Hostile || !EnemyChar->IsAlive())
				{
					OutActors.RemoveAt(i);
				}
				else if(EnemyChar != nullptr)
				{
					// check for alertness
					const float* A = Alertness.Find(EnemyChar);
					if (A == nullptr || (*A < 2.f))
					{
						OutActors.RemoveAt(i);
					}
				}
			}
		}
		if (OutActors.Num() > 0)
		{
			OutActors.Sort([&](const AActor& Test1, const AActor& Test2)
			{
				const float SqDist1 = (GetPawn()->GetActorLocation() - Test1.GetActorLocation()).SizeSquared();
				const float SqDist2 = (GetPawn()->GetActorLocation() - Test2.GetActorLocation()).SizeSquared();
				return SqDist1 < SqDist2;
			});
		}
	}
}

void ATacticalAIController::GetPercievedDamageOrigins(TArray<FVector>& OutVectors, TArray<AActor*>& OutActors,float MaxAge) const
{
	// if we have no pawn, don't even bother
	if (!GetPawn())
		return;

	// Setup Array of Damager sources
	TArray<AActor*> Damagers;
	//AIPerceptionComp->GetPerceivedActors(DamageConfig->GetSenseImplementation(), Damagers);
	AIPerceptionComp->GetCurrentlyPerceivedActors(DamageConfig->GetSenseImplementation(), Damagers);
	if (Damagers.Num() > 0)
	{
		for (AActor* TestActor : Damagers)
		{
			if (const FActorPerceptionInfo* TestPerception = AIPerceptionComp->GetActorInfo(*TestActor))
			{
				if (!TestPerception->bIsHostile)
				{
					continue;
				}

				for (const FAIStimulus& TestStimulus : TestPerception->LastSensedStimuli)
				{
					if (TestStimulus.Type == UAISense::GetSenseID(DamageConfig->GetSenseImplementation()))
					{
						if (MaxAge <= 0.f || TestStimulus.GetAge() <= MaxAge)
						{
							OutVectors.Add(TestStimulus.StimulusLocation);
						}
					}
				}
			}
		}
	}


	if (OutVectors.Num() > 1)
	{
		for (int32 i = 1; i < OutVectors.Num(); i++)
		{
			if (!OutVectors.IsValidIndex(i) || !OutVectors.IsValidIndex(i-1))
				continue;

			const int32 a = i - 1;
			const int32 b = i;

			const FVector& PawnLoc = GetPawn()->GetActorLocation();
			FVector& A = OutVectors[a];
			FVector& B = OutVectors[b];

			if (FVector::DistSquared(A, PawnLoc) > FVector::DistSquared(B, PawnLoc))
			{
				// switch A and B
				const FVector CachedLoc = A;
				A = B;
				B = CachedLoc;
			}
		}
	}
}

void ATacticalAIController::GetPercievedHearingOrigins(TArray<FVector>& OutVectors, TArray<AActor*>& OutActors, float MaxAge /*= 0.f*/, TSubclassOf<AActor> ActorClassFilter /*=nullptr*/) const
{
	OutVectors.Empty();

	// if we have no pawn, don't even bother
	if (!GetPawn())
		return;

	// Setup Array of Damager sources
	TArray<AActor*> Actors;
	//AIPerceptionComp->GetPerceivedActors(DamageConfig->GetSenseImplementation(), Damagers);
	AIPerceptionComp->GetCurrentlyPerceivedActors(HearingConfig->GetSenseImplementation(), Actors);
	if (Actors.Num() > 0)
	{
		for (AActor* TestActor : Actors)
		{
			if (!TestActor->IsValidLowLevelFast())
			{
				continue;
			}

			if (ActorClassFilter != nullptr && !TestActor->GetClass()->IsChildOf(ActorClassFilter))
			{
				continue;
			}

			if (const FActorPerceptionInfo* TestPerception = AIPerceptionComp->GetActorInfo(*TestActor))
			{
				
				if (!TestPerception->bIsHostile)
				{
					continue;
				}

				for (const FAIStimulus& TestStimulus : TestPerception->LastSensedStimuli)
				{
					if (TestStimulus.Type == UAISense::GetSenseID(HearingConfig->GetSenseImplementation()))
					{
						if (MaxAge <= 0.f || TestStimulus.GetAge() <= MaxAge)
						{
							OutVectors.Add(TestStimulus.StimulusLocation);
							OutActors.Add(TestActor);
						}
					}
				}
			}
		}
	}


	if (OutVectors.Num() > 1)
	{
		for (int32 i = 1; i < OutVectors.Num(); i++)
		{
			if (!OutVectors.IsValidIndex(i) || !OutVectors.IsValidIndex(i - 1))
				continue;

			const int32 a = i - 1;
			const int32 b = i;

			const FVector& PawnLoc = GetPawn()->GetActorLocation();
			FVector& A = OutVectors[a];
			FVector& B = OutVectors[b];

			if (FVector::DistSquared(A, PawnLoc) > FVector::DistSquared(B, PawnLoc))
			{
				// switch A and B
				const FVector CachedLoc = A;
				A = B;
				B = CachedLoc;
			}
		}
	}
}

void ATacticalAIController::GetPercievedTeamNotifications(TArray<FVector>& OutVectors, float MaxAge /*= 0.f*/) const
{
	OutVectors.Empty();

	// if we have no pawn, don't even bother
	if (!GetPawn())
		return;
	// Setup Array of Damager sources
	TArray<AActor*> PerceivedTeamActors;

	AIPerceptionComp->GetCurrentlyPerceivedActors(TeamConfig->GetSenseImplementation(), PerceivedTeamActors);
	for (AActor* TestActor : PerceivedTeamActors)
	{
		if (const FActorPerceptionInfo* TestPerception = AIPerceptionComp->GetActorInfo(*TestActor))
		{
			for (const FAIStimulus& TestStimulus : TestPerception->LastSensedStimuli)
			{
				if (TestStimulus.Type == UAISense::GetSenseID(TeamConfig->GetSenseImplementation()))
				{
					if (MaxAge <= 0.f || TestStimulus.GetAge() <= MaxAge)
					{
						OutVectors.Add(TestStimulus.StimulusLocation);
					}
					continue;
				}
			}
		}
	}
	if (OutVectors.Num() > 1)
	{
		for (int32 i = 1; i < OutVectors.Num(); i++)
		{
			if (!OutVectors.IsValidIndex(i) || !OutVectors.IsValidIndex(i - 1))
				continue;

			const FVector& PawnLoc = GetPawn()->GetActorLocation();
			FVector& A = OutVectors[i - 1];
			FVector& B = OutVectors[i];

			if (FVector::DistSquared(A, PawnLoc) > FVector::DistSquared(B, PawnLoc))
			{
				// switch A and B
				const FVector CachedLoc = A;
				A = B;
				B = CachedLoc;
			}
		}
	}
}

bool ATacticalAIController::GetBestDamageOrigin(FVector& OutVector, float MaxCost, float MaxAge) const
{
	if (GetPawn() == nullptr)
		return false;

	TArray<FVector> DamageLocs;
	TArray<AActor*> DamageActors;
	GetPercievedDamageOrigins(DamageLocs, DamageActors, MaxAge);

	bool bDirectDamage = true;

	if (DamageLocs.Num() <= 0)
	{
		bDirectDamage = false;
		GetPercievedTeamNotifications(DamageLocs, MaxAge);
	}

	if (DamageLocs.Num() <= 0)
		return false;

	if (!bDirectDamage)
	{
		for (const FVector& Loc : DamageLocs)
		{
			FCollisionQueryParams TraceParams = FCollisionQueryParams("ViewTrace", false, this);
			if (GetWorld()->LineTraceTestByChannel(GetPawn()->GetActorLocation(), Loc, TRACE_AIVision, TraceParams))
			{
				// We hit something. So we can't just shoot and attack. See if walking is worth it.
				float Cost = -1.f;
				ENavigationQueryResult::Type NavResult = UNavigationSystemV1::GetPathCost(GetWorld(), GetPawn()->GetActorLocation(), Loc, Cost);

				if (NavResult == ENavigationQueryResult::Success && Cost <= MaxCost && Cost >= 0.f)
				{
					OutVector = Loc;
					return true;
				}
			}
			else
			{
				OutVector = Loc;
				return true;
			}
		}
	}
	else
	{
		OutVector = DamageLocs[0];
		return true;
	}
	return false;
}

bool ATacticalAIController::GetBestHearingOrigin(FVector& OutVector, float MaxCost, float MaxAge /*= 0.f*/) const
{
	if (GetPawn() == nullptr)
		return false;

	TArray<FVector> HearingLocs;
	TArray<AActor*> HearingActors;
	GetPercievedHearingOrigins(HearingLocs, HearingActors, MaxAge, ATacticalCharacter::StaticClass());


	if (HearingLocs.Num() <= 0)
		return false;

	//if (!bDirectDamage)
	{
		for (const FVector& Loc : HearingLocs)
		{
			FCollisionQueryParams TraceParams = FCollisionQueryParams("ViewTrace", false, this);
			if (GetWorld()->LineTraceTestByChannel(GetPawn()->GetActorLocation(), Loc, TRACE_AIVision, TraceParams))
			{
				// We hit something. So we can't just shoot and attack. See if walking is worth it.
				float Cost = -1.f;
				ENavigationQueryResult::Type NavResult = UNavigationSystemV1::GetPathCost(GetWorld(), GetPawn()->GetActorLocation(), Loc, Cost, nullptr, HearingNavQueryFilter);

				if (NavResult == ENavigationQueryResult::Success && Cost <= MaxCost && Cost >= 0.f)
				{
					OutVector = Loc;
					return true;
				}
			}
			else
			{
				OutVector = Loc;
				return true;
			}
		}
	}

	return false;
}

bool ATacticalAIController::GetBestTeamOrigin(FVector& OutVector, float MaxCost, float MaxAge /*= 0.f*/) const
{
	if (GetPawn() == nullptr)
		return false;

	TArray<FVector> TeamLocs;
	TArray<AActor*> TeamPerceivedActors;
	GetPercievedTeamNotifications(TeamLocs, MaxAge);


	if (TeamLocs.Num() <= 0)
		return false;

	//if (!bDirectDamage)
	{
		for (const FVector& Loc : TeamLocs)
		{
			FCollisionQueryParams TraceParams = FCollisionQueryParams("ViewTrace", false, this);
			if (GetWorld()->LineTraceTestByChannel(GetPawn()->GetActorLocation(), Loc, TRACE_AIVision, TraceParams))
			{
				// We hit something. So we can't just shoot and attack. See if walking is worth it.
				float Cost = -1.f;
				ENavigationQueryResult::Type NavResult = UNavigationSystemV1::GetPathCost(GetWorld(), GetPawn()->GetActorLocation(), Loc, Cost, nullptr, HearingNavQueryFilter);

				if (NavResult == ENavigationQueryResult::Success && Cost <= MaxCost && Cost >= 0.f)
				{
					OutVector = Loc;
					return true;
				}
			}
			else
			{
				OutVector = Loc;
				return true;
			}
		}
	}

	return false;
}

void ATacticalAIController::GetPerceivedGrenades(TArray<class ATacticalProjectile*>& OutGrenades) const
{
	TArray<AActor*> PerceptionActors;
	OutGrenades.Empty();

	if (AIPerceptionComp)
	{
		//AIPerceptionComp->GetPerceivedActors(SightConfig->GetSenseImplementation(), PerceptionActors);
		//AIPerceptionComp->GetPerceivedActors(HearingConfig->GetSenseImplementation(), PerceptionActors);

		AIPerceptionComp->GetCurrentlyPerceivedActors(SightConfig->GetSenseImplementation(), PerceptionActors);
		AIPerceptionComp->GetCurrentlyPerceivedActors(HearingConfig->GetSenseImplementation(), PerceptionActors);

		for (int32 i = PerceptionActors.Num() - 1; i >= 0; --i)
		{
			if (PerceptionActors.IsValidIndex(i))
			{
				ATacticalProjectile* Projectile = Cast<ATacticalProjectile>(PerceptionActors[i]);
				if (Projectile != nullptr && Projectile->ShouldAIRun())
				{
					OutGrenades.Add(Projectile);
				}
			}
		}
		if (OutGrenades.Num() > 0)
		{
			OutGrenades.Sort([&](const AActor& Test1, const AActor& Test2)
			{
				const float SqDist1 = (GetPawn()->GetActorLocation() - Test1.GetActorLocation()).SizeSquared();
				const float SqDist2 = (GetPawn()->GetActorLocation() - Test2.GetActorLocation()).SizeSquared();
				return SqDist1 < SqDist2;
			});
		}
	}
}


AActor* ATacticalAIController::GetBestChaseActor(float MaxCost, float MaxAge) const
{
	if (!GetPawn())
	{
		return nullptr;
	}

	const FVector PawnLoc = GetPawn()->GetActorLocation();
	const UAIPerceptionComponent* PerceptionComp = AIPerceptionComp;

	// first check for immediate threats like actors who have damaged you
	TArray<AActor*> DamageActors;
	PerceptionComp->GetCurrentlyPerceivedActors(DamageConfig->GetSenseImplementation(), DamageActors);
	
	//float BestDistanceSq = -1.f;
	//float BestAge = -1.f;
	float BestScore = -1.f;	// Lower is better
	AActor *BestActor = nullptr;
	for (AActor* TestActor : DamageActors)
	{
		if (TestActor == nullptr)
		{
			continue;
		}

		float TestAge = -1.f;
		const FActorPerceptionInfo* TestInfo = PerceptionComp->GetActorInfo(*TestActor);
		
		if (!TestInfo->bIsHostile)
		{
			continue;
		}
		
		const FVector TestLoc = TestInfo->GetLastStimulusLocation(&TestAge);
		const float TestDist = FVector::DistSquared(TestLoc, PawnLoc);

		// filter out too old entries
		if (TestAge > MaxAge)
		{
			continue;
		}
		
		const float TestScore = TestDist;

		if ((BestActor == nullptr) || (TestScore < BestScore))
		{
			//BestDistanceSq = TestDist;
			//BestAge = TestAge;
			BestScore = TestScore;
			BestActor = TestActor;
		}
	}

	if (BestActor != nullptr) // todo maybe add an score threshold where we consider the result insufficient.
	{
		return BestActor;
	}

	// we weren't able to find a good Actor that damaged you.
	TArray<AActor*> PerceivedActors;
	AIPerceptionComp->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);
	for (AActor* TestActor : PerceivedActors)
	{
		if ((TestActor != nullptr) || (DamageActors.Find(TestActor) == INDEX_NONE))
		{
			continue;
		}

		float TestAge = -1.f;
		const FActorPerceptionInfo* TestInfo = PerceptionComp->GetActorInfo(*TestActor);

		if (!TestInfo->bIsHostile)
		{
			continue;
		}

		const FVector TestLoc = TestInfo->GetLastStimulusLocation(&TestAge);
		const float TestDist = FVector::DistSquared(TestLoc, PawnLoc);

		// filter out too old entries
		if (TestAge > MaxAge)
		{
			continue;
		}


	}

	return nullptr;
}


bool ATacticalAIController::IsFocusActorInSight() const
{
	AActor* FocusActor = GetFocusActor();
	if (FocusActor != nullptr)
	{
		const FActorPerceptionInfo* FocusPerception = AIPerceptionComp->GetActorInfo(*GetFocusActor());
		if(FocusPerception)
		{
			for (const FAIStimulus& TestStimulus : FocusPerception->LastSensedStimuli)
			{
				if (TestStimulus.Type == UAISense::GetSenseID(SightConfig->GetSenseImplementation()))
				{
					return TestStimulus.WasSuccessfullySensed();
				}
			}
		}
	}
	return false;
}

void ATacticalAIController::NotifyTeamInRange(float Range, AActor* NotifyActor)
{
	if (NotifyActor == nullptr || Range <= SMALL_NUMBER)
		return;

	if (Role == ROLE_Authority)
	{
		if (UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(GetWorld()))
		{
			FAITeamStimulusEvent TeamNotifyEvent(this, NotifyActor, NotifyActor->GetActorLocation(), Range);
			TeamNotifyEvent.TeamIdentifier = FGenericTeamId(GetGenericTeamId());
			PerceptionSystem->OnEvent(TeamNotifyEvent);
		}
	}
}

bool ATacticalAIController::GetActorPerceptionLocation(AActor* TrackingActor, FVector& OutLocation, float& OutAge)
{
	if (TrackingActor != nullptr)
	{
		const FActorPerceptionInfo* Perception = GetPerceptionComponent()->GetActorInfo(*TrackingActor);
		if (Perception)
		{
			float Age = 0.f;
			OutLocation = Perception->GetLastStimulusLocation(&Age);
			OutAge = Age;
			return true;
		}
	}

	return false;
}

class ATacticalCharacter* ATacticalAIController::GetTacticalCharacter() const
{
	if (GetPawn())
	{
		return Cast<ATacticalCharacter>(GetPawn());
	}
	return nullptr;
}

void ATacticalAIController::OnStartChasingLastTarget()
{
	bIsIdle = false;
}

void ATacticalAIController::OnStartAttackingTarget()
{
	bIsIdle = false;
}

void ATacticalAIController::OnReturnToIdle()
{
	bIsIdle = true;
}

float ATacticalAIController::GetAimConfidence() const
{
	if (GetPawn() && GetFocusActor())
	{
		if (const ATacticalCharacter* MyChar = Cast<ATacticalCharacter>(GetPawn()))
		{
			if (const ATacticalWeapon* WPN = MyChar->GetWeapon())
			{
				// Spread relative to maximum possible spread. 0 = best, 1 = worst.
				const float SpreadPerc = (WPN->GetSpread() - WPN->GetMinSpread()) / (WPN->GetMaxSpread() + WPN->GetMaxMovementSpread());

				// Angle to Target
				const FRotator RotToTarget = (GetFocusActor()->GetActorLocation() - MyChar->GetActorLocation()).Rotation();
				const FRotator CharacterRot = MyChar->GetViewRotation();
				const FRotator RotDelta = (RotToTarget - CharacterRot).GetNormalized();
				const float AnglePerc = FMath::Sqrt(FMath::Square(RotDelta.Yaw) + FMath::Square(RotDelta.Pitch))/255.f;
				
				return FMath::Abs(1 - AnglePerc);
			}
		}
	}

	return 0.f;
}


int32 ATacticalAIController::GetShotCount() const
{
	if (ATacticalCharacter* myChar = GetTacticalCharacter())
	{
		if (ATacticalWeapon* WPN = myChar->GetWeapon())
		{
			UTacticalWeaponState* WPNState = WPN->GetCurrentState();
			if (UTacticalWeaponStateFiring* WPNFireState = Cast<UTacticalWeaponStateFiring>(WPNState))
			{
				return WPNFireState->GetShotCount();
			}
		}
	}
	return -1;
}

void ATacticalAIController::SetWantsToCover(bool newWantstoCover)
{
	if (GetPawn())
	{
		ATacticalCharacter* myChar = Cast<ATacticalCharacter>(GetPawn());
		if(myChar)
		{
			myChar->bAIWantsToCover = newWantstoCover;
		}
	}
}

bool ATacticalAIController::WantsToCover() const
{
	if (GetPawn())
	{
		ATacticalCharacter* myChar = Cast<ATacticalCharacter>(GetPawn());
		if (myChar)
		{
			return myChar->bAIWantsToCover;
		}
	}
	return false;
}

void ATacticalAIController::SetWantsToCoverFire(bool newVal)
{
	if (GetPawn())
	{
		ATacticalCharacter* myChar = Cast<ATacticalCharacter>(GetPawn());
		if (myChar)
		{
			myChar->SetAIWantsToCoverFire(newVal);
		}
	}
}

bool ATacticalAIController::WantsToCoverFire() const
{
	if (GetPawn())
	{
		ATacticalCharacter* myChar = Cast<ATacticalCharacter>(GetPawn());
		if (myChar)
		{
			return myChar->GetAIWantsToCoverFire();
		}
	}
	return false;
}

void ATacticalAIController::OnEnteredCover()
{
	bCanCover = true;
}

void ATacticalAIController::OnLeftCover()
{
	bCanCover = false;
}


void ATacticalAIController::SetWantsToSprint(bool newWantsToSprint)
{
	if (!GetPawn())
	{
		return;
	}
	if (ATacticalCharacter* TacticalChar = Cast<ATacticalCharacter>(GetPawn()))
	{
		if (newWantsToSprint)
		{
			TacticalChar->StartSprint();
		}
		else
		{
			TacticalChar->StopSprint();
		}
	}
}

void ATacticalAIController::SetDesiredSpeedModifier(float newSpeedMod)
{
	if (!GetPawn())
	{
		return;
	}
	if (ATacticalCharacter* TacticalChar = Cast<ATacticalCharacter>(GetPawn()))
	{
		TacticalChar->SetWalkSpeedFactor(newSpeedMod);
	}
}

bool ATacticalAIController::IsGrenadeInProximity(float Range/*=500.f*/) const
{
	if (GetPawn())
	{
		TArray<ATacticalProjectile*> Grenades;
		GetPerceivedGrenades(Grenades);
		if (Grenades.Num() > 0)
		{
			// only check closest grenade
			ATacticalProjectile* Gren = Grenades[0];
			if (Gren)
			{
				const float DistSq = FVector::DistSquared(GetPawn()->GetActorLocation(), Gren->GetActorLocation());
				return (DistSq <= FMath::Square(Range));
			}
		}
	}
	return false;
}



FVector ATacticalAIController::GetAimLocation() const
{
	if (GetFocusActor() == nullptr)
	{
		return FVector::ZeroVector;
	}

	APawn* FocusPawn = Cast<APawn>(GetFocusActor());
	if (FocusPawn == nullptr)
	{
		return GetFocusActor()->GetActorLocation();
	}

	if (bWantsToMiss)
	{
		// We want to miss
		const float SpeedThreshold = 20.f;
		const FVector FocusPawnLoc = FocusPawn->GetActorLocation();

		if ((FocusPawn->GetVelocity()).SizeSquared() > SpeedThreshold)
		{
			// Get Velocity with Z = 0
			const FVector Velocity2D = FocusPawn->GetVelocity() * FVector(1.f, 1.f, 0.f);

			const float PredictTime = 0.5f; // time to predict in s
			const FVector PredictLoc = FocusPawnLoc + Velocity2D * PredictTime;

		}
	}

	return FocusPawn->GetActorLocation();
}

int32 ATacticalAIController::GetBurstCount() const
{
	ATacticalCharacter* myChar = GetTacticalCharacter();
	if(myChar)
	{
		ATacticalWeapon* WPN = myChar->GetWeapon();
		if (WPN)
		{
			const float fBurstRand = FMath::RandRange(WPN->GetAIBurstCount().Min, WPN->GetAIBurstCount().Max);
			return FMath::RoundToInt(fBurstRand);
		}
	}
	return 0;
}

float ATacticalAIController::GetBurstInterval() const
{
	ATacticalCharacter* myChar = GetTacticalCharacter();
	if (myChar)
	{
		ATacticalWeapon* WPN = myChar->GetWeapon();
		if (WPN)
		{
			const float fBurstRand = FMath::RandRange(WPN->GetAIBurstInterval().Min, WPN->GetAIBurstInterval().Max);
			return fBurstRand;
		}
	}
	return -1.f;
}

void ATacticalAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn /* = true */)
{
	// Look toward focus
	FVector FocalPoint = GetFocalPoint();

	//DrawDebugBox(GetWorld(), FocalPoint, FVector(10.f, 10.f, 10.f), FQuat(), FColor::Red, false);

	if (GetPawn())
	{
		
		FVector Direction = FAISystem::IsValidLocation(FocalPoint) ? (FocalPoint - GetPawn()->GetPawnViewLocation()) : GetPawn()->GetActorForwardVector();
		FRotator NewControlRotation = Direction.Rotation();

		// Don't pitch view unless looking at another pawn
		if ((!GetFocusActor()) || (Cast<APawn>(GetFocusActor()) == nullptr))
		{
			NewControlRotation.Pitch = 0.f;
		}

		if (GetFocusActor() /* && (Cast<APawn>(GetFocusActor()) != nullptr)*/)
		{
			// closed loop controller algorithm. Sub stepped to a maximum tick time of 0.01 s (~100FPS). if a frame needs 0.02s to render it will mean that this will take 2 iterations.
			//NewControlRotation.Yaw = FRotator::ClampAxis(NewControlRotation.Yaw);
			NewControlRotation.Yaw = FRotator::NormalizeAxis(NewControlRotation.Yaw);
			
			float remainingTime = DeltaTime;
			while (remainingTime > 0.f)
			{
				float timeTick = FMath::Min(DeltaTime, 0.01f);
				remainingTime -= timeTick;

				// PI Rotation Controller
				const FVector2D RotError = FVector2D(FRotator::NormalizeAxis(NewControlRotation.Pitch - GetControlRotation().Pitch), FRotator::NormalizeAxis(NewControlRotation.Yaw - GetControlRotation().Yaw));
				RotationErrorIntegrator += RotError * timeTick;
				FVector2D DesiredRotSpeed = RotError * KP_Aim + RotationErrorIntegrator * KI_Aim;

				const float MaxAimRotationSpeed_Current = GetTacticalCharacter()->IsFiring() ? MaxAimRotationSpeed * 0.1f : MaxAimRotationSpeed;

				DesiredRotSpeed = DesiredRotSpeed.GetSafeNormal() * FMath::Min(DesiredRotSpeed.Size(), MaxAimRotationSpeed_Current); // clamp speed

				// PI Rotation Speed Controller
				const FVector2D RotSpeedError = DesiredRotSpeed - AimVelocity;
				RotationSpeedErrorIntegrator += RotSpeedError * timeTick;
				FVector2D DesiredRotAccel = RotSpeedError * KP_AimSpeed + RotationSpeedErrorIntegrator * KI_AimSpeed;
				DesiredRotAccel = DesiredRotAccel.GetSafeNormal() * FMath::Min(DesiredRotAccel.Size(), MaxAimRotationAccel); // clamp acceleration

				AimVelocity = (AimVelocity + DesiredRotAccel * timeTick);	// apply acceleration (controller output clamped to maxaccel
				AimVelocity = AimVelocity.GetSafeNormal() * FMath::Min(AimVelocity.Size(), MaxAimRotationSpeed); // clamp to max speed

				if (!AimVelocity.IsNearlyZero(1.f))
				{
					SetControlRotation(GetControlRotation() + FRotator(AimVelocity.X * timeTick, AimVelocity.Y * timeTick, 0.f)); // apply delta
				}
			}
		}
		else
		{
			if (GetControlRotation().Equals(NewControlRotation, 1e-3f) == false && ((GetPawn()->GetActorLocation() - FocalPoint).SizeSquared2D() > 3e+4f))
			{
				NewControlRotation = FMath::RInterpTo(GetControlRotation(), NewControlRotation, DeltaTime, 10.f);
				SetControlRotation(NewControlRotation);
			}
		}

		if (bUpdatePawn)
		{
			// make pawn face the new rotation
			GetPawn()->FaceRotation(GetControlRotation(), DeltaTime);
		}
	}
}



void ATacticalAIController::OnSenseUpdate(const TArray<AActor*>& SensedActors)
{
	IsFocusActorInSight();
	//for(SensedActors)
	TArray<AActor*> SensedDamage;
	//AIPerceptionComp->GetPerceivedActors(DamageConfig->GetSenseImplementation(), SensedDamage);
	AIPerceptionComp->GetCurrentlyPerceivedActors(DamageConfig->GetSenseImplementation(), SensedDamage);
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s: Damage: %d"), *GetName(), SensedDamage.Num()));
}

void ATacticalAIController::SetAlertness(float NewValue)
{
	//Alertness = FMath::Max(NewValue, MinAlertness);

	//SightConfig->PeripheralVisionAngleDegrees = FMath::Lerp(45.f, 90.f, FMath::Clamp(Alertness, 0.f, 1.f));
	//AIPerceptionComp->ConfigureSense(*SightConfig);
}


void ATacticalAIController::InitializeBlackboardKeys_Implementation()
{}

bool ATacticalAIController::ReserveCover(FVector CoverLocation)
{
	bool bSuccess = false;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Helper);
	FCollisionQueryParams QueryParams(FName("OVERLAP_Cover"), false);
	FCollisionShape Shape = FCollisionShape::MakeSphere(50.f);
	
	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, CoverLocation, FQuat(), ObjectParams, Shape, QueryParams);
	if (Overlaps.Num() > 0)
	{
		for (FOverlapResult& TestOverlap : Overlaps)
		{
			if (TestOverlap.Actor.IsValid())
			{
				ATacticalCoverMarker* TestCover = Cast<ATacticalCoverMarker>(TestOverlap.GetActor());
				if (TestCover && TestCover->ReserveCover(GetPawn(), CoverLocation))
				{
					bSuccess = true;
				}
			}
		}
	}
	return bSuccess;
}

bool ATacticalAIController::CanReserveCover(FVector CoverLocation)
{
	bool bSuccess = false;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Helper);
	FCollisionQueryParams QueryParams(FName("OVERLAP_Cover"), false);
	FCollisionShape Shape = FCollisionShape::MakeSphere(50.f);

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, CoverLocation, FQuat(), ObjectParams, Shape, QueryParams);
	if (Overlaps.Num() > 0)
	{
		for (FOverlapResult& TestOverlap : Overlaps)
		{
			if (TestOverlap.Actor.IsValid())
			{
				ATacticalCoverMarker* TestCover = Cast<ATacticalCoverMarker>(TestOverlap.GetActor());
				if (TestCover && TestCover->IsCoverLocationFree(GetPawn(), CoverLocation))
				{
					bSuccess = true;
				}
			}
		}
	}
	return bSuccess;
}
