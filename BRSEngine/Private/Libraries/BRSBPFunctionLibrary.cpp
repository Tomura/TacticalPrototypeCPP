// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "GameFramework/PlayerInput.h"
#include "Animation/AnimMontage.h"
#include "GameMapsSettings.h"
#include "BRSMenuGameMode_Base.h"
#include "BRSPlayerController_Base.h"
#include "BRSPerlinNoiseStream.h"
#include "BehaviorTree/BTNode.h"
#include "BRSBPFunctionLibrary.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTFunctionLibrary.h"


void UBRSBPFunctionLibrary::GetKeyForAction(const FName& Action, bool bIsGamepadKey, FKey& Key)
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	for (FInputActionKeyMapping TestMapping : InputSettings->ActionMappings)
	{
		if (TestMapping.ActionName.Compare(Action) == 0 && (bIsGamepadKey == TestMapping.Key.IsGamepadKey()))
		{
			Key = TestMapping.Key;
			return;
		}
	}
	Key = EKeys::Invalid;
}

void UBRSBPFunctionLibrary::GetCharacterForKey(const FKey& Key, FString& Character)
{

}

UObject* UBRSBPFunctionLibrary::BP_GetDefaultObject(UClass* inClass)
{
	if (inClass)
	{
		return inClass->GetDefaultObject();
	}
	return nullptr;
}

float UBRSBPFunctionLibrary::GetAnimSequenceLength(UAnimSequence* Anim)
{
	return Anim->SequenceLength * Anim->RateScale;
}

float UBRSBPFunctionLibrary::GetAnimMontageLength(class UAnimMontage* Montage)
{
	return Montage->CalculateSequenceLength();
}

float UBRSBPFunctionLibrary::GetAnimMontageSectionLength(class UAnimMontage* Montage, int32 Section)
{
	return Montage->GetSectionLength(Section);
}

void UBRSBPFunctionLibrary::SetupNavLinkProxy(class ANavLinkProxy* NavLink, const FVector& V1, const FVector& V2)
{
	//NavLink->Link
	//UNavLinkCustomComponent* LinkComp = NavLink->GetSmartLinkComp();
	//LinkComp->Add
}

bool UBRSBPFunctionLibrary::IsMainMenu()
{
	return (GWorld && GWorld->GetAuthGameMode() && Cast<ABRSMenuGameMode_Base>(GWorld->GetAuthGameMode()));
}

bool UBRSBPFunctionLibrary::IsUsingGamepad(APlayerController* PC)
{
	ABRSPlayerController_Base* BRSPC = Cast<ABRSPlayerController_Base>(PC);
	if (BRSPC)
	{
		return BRSPC->IsUsingGamepad();
	}
	return false;
}

//bool UBRSBPFunctionLibrary::HasGamepad()
//{
//}


/** @RETURN True if weapon trace from Origin hits component VictimComp.  OutHitResult will contain properties of the hit. */
static bool BRSComponentIsDamageableFrom(UPrimitiveComponent* VictimComp, FVector const& Origin, AActor const* IgnoredActor, const TArray<AActor*>& IgnoreActors, ECollisionChannel TraceChannel, FHitResult& OutHitResult)
{
	static FName NAME_ComponentIsVisibleFrom = FName(TEXT("ComponentIsVisibleFrom"));
	FCollisionQueryParams LineParams(NAME_ComponentIsVisibleFrom, true, IgnoredActor);
	LineParams.AddIgnoredActors(IgnoreActors);

	// Do a trace from origin to middle of box
	UWorld* const World = VictimComp->GetWorld();
	check(World);

	FVector const TraceEnd = VictimComp->Bounds.Origin;
	FVector TraceStart = Origin;
	if (Origin == TraceEnd)
	{
		// tiny nudge so LineTraceSingle doesn't early out with no hits
		TraceStart.Z += 0.01f;
	}
	bool const bHadBlockingHit = World->LineTraceSingleByChannel(OutHitResult, TraceStart, TraceEnd, TraceChannel, LineParams);
	//::DrawDebugLine(World, TraceStart, TraceEnd, FLinearColor::Red, true);

	// If there was a blocking hit, it will be the last one
	if (bHadBlockingHit)
	{
		if (OutHitResult.Component == VictimComp)
		{
			// if blocking hit was the victim component, it is visible
			return true;
		}
		else
		{
			// if we hit something else blocking, it's not
			UE_LOG(LogDamage, Log, TEXT("Radial Damage to %s blocked by %s (%s)"), *GetNameSafe(VictimComp), *GetNameSafe(OutHitResult.GetActor()), *GetNameSafe(OutHitResult.Component.Get()));
			return false;
		}
	}

	// didn't hit anything, assume nothing blocking the damage and victim is consequently visible
	// but since we don't have a hit result to pass back, construct a simple one, modeling the damage as having hit a point at the component's center.
	FVector const FakeHitLoc = VictimComp->GetComponentLocation();
	FVector const FakeHitNorm = (Origin - FakeHitLoc).GetSafeNormal();		// normal points back toward the epicenter
	OutHitResult = FHitResult(VictimComp->GetOwner(), VictimComp, FakeHitLoc, FakeHitNorm);
	return true;
}


bool UBRSBPFunctionLibrary::ApplyRadialDamageWithFalloff(UObject* WorldContextObject, float BaseDamage, float MinimumDamage, const FVector& Origin, float DamageInnerRadius, float DamageOuterRadius, float DamageFalloff, TSubclassOf<class UDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor* DamageCauser, AController* InstigatedByController, ECollisionChannel DamagePreventionChannel)
{
	static FName NAME_ApplyRadialDamage = FName(TEXT("ApplyRadialDamage"));
	FCollisionQueryParams SphereParams(NAME_ApplyRadialDamage, false);

	SphereParams.AddIgnoredActors(IgnoreActors);

	// query scene to see what we hit
	TArray<FOverlapResult> Overlaps;
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	World->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(DamageOuterRadius), SphereParams);

	// collate into per-actor list of hit components
	TMap<AActor*, TArray<FHitResult> > OverlapComponentMap;
	for (int32 Idx = 0; Idx<Overlaps.Num(); ++Idx)
	{
		FOverlapResult const& Overlap = Overlaps[Idx];
		AActor* const OverlapActor = Overlap.GetActor();

		if (OverlapActor &&
			OverlapActor->bCanBeDamaged &&
			Overlap.Component.IsValid())
		{
			FHitResult Hit;
			if (DamagePreventionChannel == ECC_MAX || BRSComponentIsDamageableFrom(Overlap.Component.Get(), Origin, nullptr, IgnoreActors, DamagePreventionChannel, Hit))
			{
				TArray<FHitResult>& HitList = OverlapComponentMap.FindOrAdd(OverlapActor);
				HitList.Add(Hit);
			}
		}
	}

	// make sure we have a good damage type
	TSubclassOf<UDamageType> const ValidDamageTypeClass = (DamageTypeClass == NULL) ? TSubclassOf<UDamageType>(UDamageType::StaticClass()) : DamageTypeClass;

	bool bAppliedDamage = false;

	// call damage function on each affected actors
	for (TMap<AActor*, TArray<FHitResult> >::TIterator It(OverlapComponentMap); It; ++It)
	{
		AActor* const Victim = It.Key();
		TArray<FHitResult> const& ComponentHits = It.Value();

		FRadialDamageEvent DmgEvent;
		DmgEvent.DamageTypeClass = ValidDamageTypeClass;
		DmgEvent.ComponentHits = ComponentHits;
		DmgEvent.Origin = Origin;
		DmgEvent.Params = FRadialDamageParams(BaseDamage, MinimumDamage, DamageInnerRadius, DamageOuterRadius, DamageFalloff);

		Victim->TakeDamage(BaseDamage, DmgEvent, InstigatedByController, DamageCauser);

		bAppliedDamage = true;
	}

	return bAppliedDamage;
}

float UBRSBPFunctionLibrary::HorizontalToVerticalFOV(float HorizontalFOV)
{
	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		return FMath::RadiansToDegrees(2 * FMath::Atan(FMath::Tan(FMath::DegreesToRadians(HorizontalFOV) * 0.5f) * ViewportSize.Y / ViewportSize.X));
	}
	return -1.f;
}

float UBRSBPFunctionLibrary::VerticalToHorizontalFOV(float VerticalFOV)
{
	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		return FMath::RadiansToDegrees(2 * FMath::Atan(FMath::Tan(FMath::DegreesToRadians(VerticalFOV) * 0.5f) * ViewportSize.X / ViewportSize.Y));
	}
	return -1.f;
}

const FText UBRSBPFunctionLibrary::GetKeyboardKeyShortName(const FKey& Key)
{
	if (Key.IsGamepadKey() || Key.IsMouseButton())
		return FText::GetEmpty();

	if (Key == EKeys::Semicolon)	return NSLOCTEXT("ShortKeyNames", "Semicolon", ";");
	if (Key == EKeys::Comma)		return NSLOCTEXT("ShortKeyNames", "Comma", ",");
	if (Key == EKeys::Hyphen)		return NSLOCTEXT("ShortKeyNames", "Hyphen", "-");
	if (Key == EKeys::Underscore)	return NSLOCTEXT("ShortKeyNames", "Underscore", "_");
	if (Key == EKeys::Period)		return NSLOCTEXT("ShortKeyNames", "Period", ".");
	if (Key == EKeys::Apostrophe)	return NSLOCTEXT("ShortKeyNames", "Apostrophe", "'");
	if (Key == EKeys::Asterix)		return NSLOCTEXT("ShortKeyNames", "Asterix", "*");
	if (Key == EKeys::Colon)		return NSLOCTEXT("ShortKeyNames", "Colon", ":");
	if (Key == EKeys::SpaceBar)		return NSLOCTEXT("ShortKeyNames", "SpaceBar", "Space");
	
	return Key.GetDisplayName();
}




FString UBRSBPFunctionLibrary::GetClassPath(UClass* InClass)
{
	return InClass->GetPathName(nullptr);
}

TSubclassOf<AGameMode> UBRSBPFunctionLibrary::GetGameModeFromShortName(const FString& ShortName)
{
	FString GameModePath = UGameMapsSettings::GetGameModeForName(ShortName);
	return LoadClass<AGameMode>(nullptr, *GameModePath);
}

FString UBRSBPFunctionLibrary::GetNetMode(APlayerController* PC)
{
	if (PC)
	{
		switch (PC->GetNetMode())
		{
		case ENetMode::NM_Standalone:
			return FString("Standalone");
		case ENetMode::NM_DedicatedServer:
			return FString("Dedicated Server");
		case ENetMode::NM_ListenServer:
			return FString("Listen Server");
		case ENetMode::NM_Client:
			return FString("Client");
		default:
			break;
		}
	}
	return FString("Failure");
}

bool UBRSBPFunctionLibrary::BlackboardKeyValid(UBTNode* NodeOwner, const FBlackboardKeySelector& Key)
{
	check(NodeOwner != NULL);
	const UBlackboardComponent* BlackboardComp = UBTFunctionLibrary::GetOwnersBlackboard(NodeOwner);

	if (BlackboardComp)
	{
		FBlackboard::FKey KeyId = BlackboardComp->GetKeyID(Key.SelectedKeyName);
		UBlackboardData* Asset = BlackboardComp->GetBlackboardAsset();
		if (Asset != nullptr)
		{
			const FBlackboardEntry* Entry = Asset->GetKey(KeyId);
			if ((Entry != nullptr) && (Entry->KeyType != nullptr))
			{
				return true;
			}
		}
	}
	return false;
}

//void UBRSGameInstance::GetSessionDataById(int32 SessionIdx, FString& MapName, TSubclassOf<AGameMode>& GameMode, int32& MaxPlayers, int32& NumPlayers, int32& Ping, bool& bPrivate, bool& bDedicated) const
//{
//	ABRSGameSession* Session = GetGameSession();
//	if (Session)
//	{
//		const TArray<FOnlineSessionSearchResult>& Results = Session->GetSearchResults();
//		if (Results.IsValidIndex(SessionIdx))
//		{
//			FOnlineSessionSearchResult ThisSession = Results[SessionIdx];
//			Ping = ThisSession.PingInMs;
//			NumPlayers = ThisSession.Session.SessionSettings.NumPublicConnections - ThisSession.Session.NumOpenPublicConnections;
//			MaxPlayers = ThisSession.Session.SessionSettings.NumPublicConnections;
//			bDedicated = ThisSession.Session.SessionSettings.bIsDedicated;
//
//		}
//	}
//}
