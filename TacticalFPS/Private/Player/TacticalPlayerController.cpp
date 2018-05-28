// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalPlayerController.h"
#include "TacticalCharacter.h"
#include "TacticalPauseMenu.h"
#include "TacticalGameMode.h"
#include "TacticalPlayerState.h"
#include "TacticalGameState.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponAttachmentPoint.h"
#include "TacticalGameplaySettings.h"
#include "TacticalWeaponPreviewScene.h"
#include "BRSMenuGameMode_Base.h"
#include "TacticalHUD.h"
#include "TacticalWeaponAttachmentPoint.h"
#include "TacticalGadgetInterface.h"
#include "TacticalLoadoutManager.h"



ATacticalPlayerController::ATacticalPlayerController(const FObjectInitializer& OI)
	: Super(OI)
{
	TeamID = FGenericTeamId(TEAM_Players);

	GameHUDClass = nullptr;
	PauseMenuClass = nullptr;

	PredictionFudgeFactor = 15.f;
	MaxPredictionPing = 200.f; //todo: have this in engine and make it more dynamic to adjust ingame

	bInMenu = false;

	SpawningLoadout = FTacticalLoadout();

	PlayerPostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("Player Post Process"));
}

void ATacticalPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetGenericTeamId(TeamID);

	if (IsLocalPlayerController())
	{
	//	if (GameHUDClass && GameHUDClass->IsValidLowLevel())
	//	{
	//		GameHUDWidget = CreateWidget<UUserWidget>(this, GameHUDClass);
	//		if (GameHUDWidget)
	//		{
	//			GameHUDWidget->AddToPlayerScreen();
	//		}
	//	}
	//	if (PauseMenuClass && PauseMenuClass->IsValidLowLevel())
	//	{
	//		PauseMenuWidget = CreateWidget<UTacticalPauseMenu>(this, PauseMenuClass);
	//		if (PauseMenuWidget)
	//		{
	//			PauseMenuWidget->AddToPlayerScreen();
	//		}
	//	}

		const float DefaultFOV = GetDefault<UTacticalGameplaySettings>(UTacticalGameplaySettings::StaticClass())->DefaultFOV;

		FOV(DefaultFOV);
	}
	else
	{
		PlayerPostProcessComp->DestroyComponent();
		PlayerPostProcessComp = nullptr;
	}
}
void ATacticalPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	ATacticalWeaponPreviewScene* WeaponPreview = GetViewTarget() ? Cast<ATacticalWeaponPreviewScene>(GetViewTarget()) : nullptr;
	if (WeaponPreview)
	{
		// use FOV from camera of preview scene. This way it can be modified properly
		float POV_FOV = WeaponPreview->GetPreviewCamera()->FieldOfView;
		POV_FOV = UBRSBPFunctionLibrary::HorizontalToVerticalFOV(POV_FOV);
		FOV(POV_FOV);
	}
	else if(GetViewTarget() && GetViewTarget()->GetClass() == ACameraActor::StaticClass())
	{
		float POV_FOV = PlayerCameraManager->ViewTarget.POV.FOV; // Horizontal FOV for 16:9 aspect
		const FVector2D ViewportSize(16.f, 9.f);
		POV_FOV = FMath::RadiansToDegrees(2 * FMath::Atan(FMath::Tan(FMath::DegreesToRadians(POV_FOV) * 0.5f) * ViewportSize.Y / ViewportSize.X)); // Convert to vertical fov

		FOV(POV_FOV);
	}
	else
	{
		const float DefaultFOV = GetDefault<UTacticalGameplaySettings>(UTacticalGameplaySettings::StaticClass())->DefaultFOV;
		if (GetPawn() && IsLocalPlayerController())
		{
			ATacticalCharacter* myChar = Cast<ATacticalCharacter>(GetPawn());
			if (myChar)
			{
				const float CurrentFOV = PlayerCameraManager->GetFOVAngle();
				const float ZoomMod = (myChar->GetWeapon() && myChar->GetWeapon()->GetAimZoomModifier() > 1.f) ?
					myChar->GetWeapon()->GetAimZoomModifier() : 1.f;
				float DesiredFOV = myChar->IsAiming() ? (DefaultFOV * (1.f / ZoomMod)) : DefaultFOV;
				if (CurrentFOV != DesiredFOV)
				{
					float NewFOV = FMath::FInterpConstantTo(CurrentFOV, DesiredFOV, DeltaTime, 140.f);
					FOV(NewFOV);
				}
			}
		}
		else
		{
			FOV(DefaultFOV);
		}
	}

	// Tick function that only runs for local player controllers
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer)
	{
		TickLocalPlayerOnly(DeltaTime);
	}
}


void ATacticalPlayerController::TickLocalPlayerOnly(float DeltaTime)
{
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (!LocalPlayer)
	{
		return;
	}

	bShowMouseCursor = IsInMenu();

	// this is pretty hacky, but works. Forces focus on what needs it. (If in game game will always have focus, if in chat chat will always have focus)
	if (!IsInMenu() && !IsInChat())
	{
		// HACK: Just using Controller id for local player index
		FSlateApplication::Get().SetUserFocusToGameViewport(LocalPlayer->GetControllerId(), EFocusCause::Cleared);
		if (GetSpectatorPawn() && !GetSpectatorPawn()->InputEnabled())
		{
			GetSpectatorPawn()->EnableInput(this);
		}
		if (GetPawn() && !GetPawn()->InputEnabled())
		{
			GetPawn()->EnableInput(this);
		}
	}
	else if (IsInChat())
	{
		GetPauseMenuWidget()->FocusChat();
		// Make sure that input is disabled
		if (GetSpectatorPawn() && GetSpectatorPawn()->InputEnabled())
		{
			GetSpectatorPawn()->DisableInput(this);
		}
		if (GetPawn() && GetPawn()->InputEnabled())
		{
			GetPawn()->DisableInput(this);
		}
	}
	else
	{
		if (GetPauseMenuWidget() && !(GetPauseMenuWidget()->HasUserFocusedDescendants(this) || GetPauseMenuWidget()->HasUserFocus(this)))
		{
			//GetPauseMenuWidget()->SetUserFocus(this);
		}

		// Make sure that input is disabled
		if (GetSpectatorPawn() && GetSpectatorPawn()->InputEnabled())
		{
			GetSpectatorPawn()->DisableInput(this);
		}
		if (GetPawn() && GetPawn()->InputEnabled())
		{
			GetPawn()->DisableInput(this);
		}
	}
}

void ATacticalPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	InputComponent->BindAction("Menu", IE_Pressed, this, &ATacticalPlayerController::ToggleMenu).bExecuteWhenPaused = true;
	FInputActionBinding& ActionStartRotRight = InputComponent->BindAction("RotatePreview", IE_Pressed, this, &ATacticalPlayerController::StartRotatePreview);
	FInputActionBinding& ActionStopRotRight = InputComponent->BindAction("RotatePreview", IE_Released, this, &ATacticalPlayerController::StopRotatePreview);

	ActionStartRotRight.bConsumeInput = false;
	ActionStopRotRight.bConsumeInput = false;
	ActionStartRotRight.bExecuteWhenPaused = true;
	ActionStopRotRight.bExecuteWhenPaused = true;

	FInputAxisBinding& ActionRotPreviewX = InputComponent->BindAxis("RotatePreviewX", this, &ATacticalPlayerController::RotatePreviewX);
	FInputAxisBinding& ActionRotPreviewY = InputComponent->BindAxis("RotatePreviewY", this, &ATacticalPlayerController::RotatePreviewY);


	ActionRotPreviewX.bConsumeInput = false;
	ActionRotPreviewY.bConsumeInput = false;
	ActionRotPreviewX.bExecuteWhenPaused = true;
	ActionRotPreviewY.bExecuteWhenPaused = true;


	FInputKeyBinding& KeyLMBBinding = InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ATacticalPlayerController::LeftMouseButtonDown);
	KeyLMBBinding.bConsumeInput = false;
	KeyLMBBinding.bExecuteWhenPaused = true;

	InputComponent->BindAction("Chat", IE_Pressed, this, &ATacticalPlayerController::OpenChat);
	InputComponent->BindAction("TeamChat", IE_Pressed, this, &ATacticalPlayerController::OpenTeamChat);
}


void ATacticalPlayerController::LeftMouseButtonDown()
{
	if (WeaponPreviewScene)
	{
		WeaponPreviewScene->OnClickedOutside();
	}
}



void ATacticalPlayerController::UnPossess()
{
	APawn* PreviousPawn = GetPawn();
	Super::UnPossess();

	UE_LOG(LogTemp, Log, TEXT("(%s) Unpossessing"), *GetFullName());
	ChangeState(NAME_Spectating);
	if (GetSpectatorPawn())
	{
		GetSpectatorPawn()->AddActorLocalOffset(FVector(0.f, 0.f, 150.f));
		if (PreviousPawn)
		{
			SetControlRotation((PreviousPawn->GetActorLocation() - GetSpectatorPawn()->GetActorLocation()).Rotation());
		}
		if (bInMenu)
		{
			GetSpectatorPawn()->DisableInput(this);
		}
	}

	ATacticalGameMode* TGM = GetWorld()->GetAuthGameMode<ATacticalGameMode>();
	if (TGM && PreviousPawn) // Also checks for old pawn, so it doesn't fire when unpossessing spectator pawn
	{
		TGM->EndRoundCheck();
	}
}

void ATacticalPlayerController::FOV(float NewFOV)
{
	//NewFOV = FMath::Clamp(NewFOV, 30.f, 170.f);
	Super::FOV(NewFOV);
}



void ATacticalPlayerController::SetDefaultFOV(float NewFOV, bool bSave)
{
	NewFOV = FMath::Clamp(NewFOV, 30.f, 170.f);

	Super::FOV(NewFOV);

	UTacticalGameplaySettings* GameplaySettings = GetMutableDefault<UTacticalGameplaySettings>(UTacticalGameplaySettings::StaticClass());
	GameplaySettings->DefaultFOV = NewFOV;

	if(bSave)
		GameplaySettings->SaveConfig();
}

float ATacticalPlayerController::GetDefaultFOV() const
{
	return GetDefault<UTacticalGameplaySettings>(UTacticalGameplaySettings::StaticClass())->DefaultFOV;
}

float ATacticalPlayerController::GetPredictionTime() const
{
	// Exact Ping is in ms, but we want to return in s
	// todo: change to return only half ping and let other half be handled by Forward Interpolation on Client
	return (PlayerState && (GetNetMode() != NM_Standalone)) ?
		(0.001f*FMath::Clamp(PlayerState->ExactPing - PredictionFudgeFactor, 0.f, MaxPredictionPing)) : 0.f;
}

bool ATacticalPlayerController::IsInMenu() const
{
	const bool bIsMenuGameMode = Super::IsInMenu();
	if (bIsMenuGameMode)
	{
		return true;
	}

	if (GetWorld() && GetWorld()->GetGameState())
	{
		if (ATacticalGameState* TGS = Cast<ATacticalGameState>(GetWorld()->GetGameState()))
		{
			if (TGS->WaitingForNextRound())
			{
				return true;
			}
		}
	}

	return bInMenu;
}

bool ATacticalPlayerController::IsInChat() const
{
	if (GetPauseMenuWidget())
	{
		return GetPauseMenuWidget()->IsInChat();
	}
	return false;
}

void ATacticalPlayerController::AddClientTravelParams(FString& URL)
{
	//URL = URL.Append(SpawningLoadout.GenerateURLParams());
}

void ATacticalPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
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

FGenericTeamId ATacticalPlayerController::GetGenericTeamId() const
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

ETeamAttitude::Type ATacticalPlayerController::GetTeamAttitudeTowards(const AActor& Other) const
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
}

void ATacticalPlayerController::OnPawnDied_Implementation(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser)
{

}

void ATacticalPlayerController::OnPawnTakeDamage_Implementation(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser)
{

}

void ATacticalPlayerController::Reset()
{
	Super::Reset();
}


void ATacticalPlayerController::ClientResetLevel_Implementation()
{
	// only run this locally not for remote controller
	UE_LOG(LogTemp, Log, TEXT("ResetLevel for Client!"));
	if (IsLocalController())
	{
		for (FObjectIterator Itr(UDecalComponent::StaticClass(), true); Itr; ++Itr)
		{
			if (*Itr == nullptr)
				continue;

			UDecalComponent* TestDecal = Cast<UDecalComponent>(*Itr);
			if (TestDecal && TestDecal->IsValidLowLevelFast())
			{
				if(TestDecal->GetWorld() != GetWorld())
					continue;

				if(TestDecal->IsPendingKill())
					continue;

				if (TestDecal->ComponentHasTag(TAG_RemoveAfterRound))
				{
					UE_LOG(LogTemp, Log, TEXT("Found Decal"));
					TestDecal->DestroyComponent();
				}
			}
		}
	}
}

bool ATacticalPlayerController::IsAlliedTo(APlayerState* OtherPlayer) const
{
	return (GetTeamAttitudeTowards(*OtherPlayer) == ETeamAttitude::Friendly);
}

void ATacticalPlayerController::ToggleMenu()
{
	ShowPauseMenu(!bInMenu);
}

void ATacticalPlayerController::ShowPauseMenu(bool bShow)
{
	if (IsInMenu() != bShow)
	{
		if (GetPauseMenuWidget() && !GetWorldTimerManager().IsTimerActive(MenuTimer))
		{
			bInMenu = bShow;
			if (GetPawn())
			{
				if (bInMenu)
				{
					GetPawn()->DisableInput(this);
				}
				else
				{
					GetPawn()->EnableInput(this);
				}
			}

			if (GetSpectatorPawn())
			{
				if (bInMenu)
				{
					GetSpectatorPawn()->DisableInput(this);
				}
				else
				{
					GetSpectatorPawn()->EnableInput(this);
				}
			}

			GetPauseMenuWidget()->OpenMenu(bInMenu);
			GetWorldTimerManager().SetTimer(MenuTimer, 0.6f, false);

			//GetPawn->Time
			//if (bShow)
			//	Pause();
			//else
			//	Pause();
			
		}
	}
}

void ATacticalPlayerController::NotifyReadyToPlay(bool bReady)
{
	if (bReady)
	{
		ServerSetLoadoutAndReady(GetLoadout());
	}
	else
	{
		ServerSetReady(false);
	}
}

void ATacticalPlayerController::ClientOpenPreRoundMenu_Implementation()
{
}

void ATacticalPlayerController::ClientForceReady_Implementation()
{
	ServerSetLoadout(SpawningLoadout);
	ServerSetReady(true);
}


void ATacticalPlayerController::ServerSetReady_Implementation(bool bReady)
{
	if (ATacticalPlayerState* TPState = Cast<ATacticalPlayerState>(PlayerState))
	{
		TPState->bReady = bReady;
		if (bReady && GetWorld()->GetAuthGameMode())
		{
			if (ATacticalGameMode* TGM = Cast<ATacticalGameMode>(GetWorld()->GetAuthGameMode()))
			{
				TGM->PlayerBecameReady(this);
			}
		}
	}
}

void ATacticalPlayerController::LocalSetLoadout(const FTacticalLoadout& NewLoadout, bool bSave)
{
	SpawningLoadout = NewLoadout; 

	if (WeaponPreviewScene)
	{
		WeaponPreviewScene->SetFromPlayer(this);
	}

	if(bSave) SaveConfig();
}

void ATacticalPlayerController::ClientSetLoadout_Implementation(FTacticalLoadout NewLoadout)
{
	SpawningLoadout = NewLoadout;
	SaveConfig();

	ServerSetLoadout(NewLoadout);
}

void ATacticalPlayerController::ServerSetLoadout_Implementation(FTacticalLoadout NewLoadout)
{
	SpawningLoadout = NewLoadout;
}


void ATacticalPlayerController::LocalSetLoadoutAttachment(class UTacticalWeaponAttachmentPoint* AttachPoint, int32 AttachmentIdx, EWeaponClass WPNType, bool bSave)
{
	if (!AttachPoint)
		return;
	const FString& AttachPointName = AttachPoint->GetName();
	LocalSetLoadoutAttachment(AttachPointName, AttachmentIdx, WPNType, bSave);
}

void ATacticalPlayerController::LocalSetLoadoutAttachment(const FString& AttachPointName, int32 AttachmentIdx, EWeaponClass WPNType, bool bSave)
{
	SpawningLoadout.SetAttachment(AttachPointName, AttachmentIdx, WPNType);

	if (WeaponPreviewScene)
	{
		WeaponPreviewScene->AddWeaponAttachment(AttachPointName, AttachmentIdx, WPNType);
	}

	if (bSave)
	{
		SaveConfig();
	}
}

void ATacticalPlayerController::LocalSetLoadoutWeapon(TSubclassOf<class ATacticalWeapon> WeaponType, bool bSave)
{
	bool bMainWeapon = false;
	if (WeaponType->IsChildOf(ATacticalPrimaryWeapon::StaticClass()))
	{
		SpawningLoadout.MainWeaponType = *WeaponType;
		SpawningLoadout.MainWPNAttachmentString.Empty();
		SpawningLoadout.MainWPN_Ammo = 0;
		bMainWeapon = true;
	}
	else if (WeaponType->IsChildOf(ATacticalSidearm::StaticClass()))
	{
		SpawningLoadout.SidearmType = *WeaponType;
		SpawningLoadout.SidearmAttachmentString.Empty();
		SpawningLoadout.Sidearm_Ammo = 0;
		bMainWeapon = false;
	}
	else
	{
		return;
	}

	EWeaponClass WPNType = (bMainWeapon ? EWeaponClass::MainWeapon : EWeaponClass::Sidearm);

	if (WeaponPreviewScene)
	{
		WeaponPreviewScene->SetWeapon(WeaponType, this, WPNType);
	}

	// Create Default Attachments
	// This iterates though BPConstruction nodes to find components created in BP Editor. We only have the template to work with so this is the complex way.
	// If this blueprint depends on another parent, we will probably have to make this even more complicated
	{	
		TArray<USCS_Node*> BPNodes;
		TArray<const UBlueprintGeneratedClass*> ParentBPClassStack;
		UBlueprintGeneratedClass::GetGeneratedClassesHierarchy(WeaponType, ParentBPClassStack);
		for (int32 i = ParentBPClassStack.Num() - 1; i >= 0; i--)
		{
			const UBlueprintGeneratedClass* CurrentBPGClass = ParentBPClassStack[i];
			if (CurrentBPGClass->SimpleConstructionScript)
			{
				BPNodes += CurrentBPGClass->SimpleConstructionScript->GetAllNodes();
			}
		}

		for (int32 i = 0; i < BPNodes.Num(); i++)
		{
			
			USceneComponent* ComponentTemplate = Cast<USceneComponent>(BPNodes[i]->GetActualComponentTemplate(Cast<UBlueprintGeneratedClass>(WeaponType)));
			if (ComponentTemplate != NULL)
			{
				if (UTacticalWeaponAttachmentPoint* AttachPointTemplate = Cast<UTacticalWeaponAttachmentPoint>(ComponentTemplate))
				{
					
					LocalSetLoadoutAttachment(BPNodes[i]->GetVariableName().ToString(), 0, WPNType, false);
				}
			}
		}
	}

	if (bSave)
	{
		SaveConfig();
	}
}


void ATacticalPlayerController::LocalSetLoadoutAmmo(int32 AmmoIdx, EWeaponClass WeaponClass, bool bSave)
{
	if (WeaponClass == EWeaponClass::MainWeapon)
	{
		if (GetLoadout().MainWeaponType == nullptr)
		{
			AmmoIdx = 0;
		}
		SpawningLoadout.MainWPN_Ammo = AmmoIdx;
	}
	else
	{
		if (GetLoadout().SidearmType == nullptr)
		{
			AmmoIdx = 0;
		}
		SpawningLoadout.Sidearm_Ammo = AmmoIdx;
	}

	if (WeaponPreviewScene)
	{
		WeaponPreviewScene->SetAmmoPreview(AmmoIdx);
	}

	if (bSave)
	{
		SaveConfig();
	}
}

void ATacticalPlayerController::LocalSetLoadoutGadget(int32 GadgetNum, TSubclassOf<class ATacticalInventory> ItemType, bool bSave)
{
	if (ItemType->ImplementsInterface(UTacticalGadgetInterface::StaticClass()))
	{
		if (GadgetNum <= 0)
		{
			SpawningLoadout.Gadget1 = *ItemType;
		}
		else
		{
			SpawningLoadout.Gadget2 = *ItemType;
		}

		if (bSave)
		{
			SaveConfig();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s does not implement gadget interface. Could not add item as gadget."), *ItemType->GetName());
	}
}


void ATacticalPlayerController::OpenChatWidget_Implementation(bool bTeamChat /*= false*/)
{
	if (GetPauseMenuWidget())
	{
		GetPauseMenuWidget()->OpenChat(bTeamChat);
	}
}

bool ATacticalPlayerController::ToggleAim() const
{
	const UTacticalGameplaySettings* TGSettings = GetDefault<UTacticalGameplaySettings>();
	if (TGSettings)
	{
		return TGSettings->bToggleAim;
	}
	return true;
}

bool ATacticalPlayerController::ToggleCrouch() const
{
	const UTacticalGameplaySettings* TGSettings = GetDefault<UTacticalGameplaySettings>();
	if (TGSettings)
	{
		return TGSettings->bToggleCrouch;
	}
	return true;
}

class ATacticalWeaponPreviewScene* ATacticalPlayerController::FocusWeaponPreview()
{
	ATacticalWeaponPreviewScene* WeaponPreview = nullptr;
	if (GetWorld())
	{
		for (TActorIterator<ATacticalWeaponPreviewScene> It(GetWorld()); It; ++It)
		{
			if (*It)
			{
				WeaponPreview = *It;
				break;
			}
		}
	}
	if (WeaponPreview && PlayerCameraManager)
	{
		WeaponPreviewScene = WeaponPreview;
		PlayerCameraManager->SetViewTarget(WeaponPreview);
		WeaponPreview->OnFocusPreview(this);
		return WeaponPreview;
	}
	return nullptr;
}



void ATacticalPlayerController::UnFocusWeaponPreview()
{
	ATacticalWeaponPreviewScene* WeaponPreview = nullptr;
	if (GetWorld())
	{
		for (TActorIterator<ATacticalWeaponPreviewScene> It(GetWorld()); It; ++It)
		{
			if (*It)
			{
				WeaponPreview = *It;
				break;
			}
		}
	}
	if (WeaponPreview)
	{
		WeaponPreview->OnUnfocusPreview(this);
		WeaponPreviewScene = nullptr;
	}

	if (GetWorld()->GetAuthGameMode())
	{
		ATacticalGameModeBase* TGM = Cast<ATacticalGameModeBase>(GetWorld()->GetAuthGameMode());
		if (TGM)
		{
			TGM->OnPlayerSetLoadout(this);
		}
	}

	if (PlayerCameraManager)
	{
		const float DefaultFOV = GetDefault<UTacticalGameplaySettings>(UTacticalGameplaySettings::StaticClass())->DefaultFOV;
		FOV(DefaultFOV);

		if (ABRSMenuGameMode_Base* MenuGM = GetWorld()->GetAuthGameMode() ? Cast<ABRSMenuGameMode_Base>(GetWorld()->GetAuthGameMode()) : nullptr)
		{
			// don't modify the viewport. This should be handled by the level script.
			return;
		}
		else if (GetPawn())
		{
			SetViewTarget(GetPawn());
			return;
		}
		else if (GetSpectatorPawn())
		{
			SetViewTarget(GetSpectatorPawn());
			return;
		} 

		PlayerCameraManager->SetViewTarget(nullptr);
	}


}


bool ATacticalPlayerController::IsUsingFreecam() const
{
	return (PlayerCameraManager ? PlayerCameraManager->CameraStyle == FName(TEXT("Freecam")) : false);
}



void ATacticalPlayerController::ServerSetLoadoutAndReady_Implementation(struct FTacticalLoadout ConfirmedLoadout)
{
	// todo: Check if loadout is allowed
	SpawningLoadout = ConfirmedLoadout;

	// Set Ready State
	if (ATacticalPlayerState* TPState = Cast<ATacticalPlayerState>(PlayerState))
	{
		TPState->bReady = true;
		if (GetWorld()->GetAuthGameMode())
		{
			if (ATacticalGameMode* TGM = Cast<ATacticalGameMode>(GetWorld()->GetAuthGameMode()))
			{
				TGM->PlayerBecameReady(this);
			}
		}
	}
}

class UTacticalPauseMenu* ATacticalPlayerController::GetPauseMenuWidget() const
{
	if (!GetHUD())
	{
		return nullptr;
	}

	if (ATacticalHUD* THUD = Cast<ATacticalHUD>(GetHUD()))
	{
		return THUD->GetPauseMenuWidget();
	}
	return nullptr;
}




class UUserWidget* ATacticalPlayerController::GetGameHUDWidget() const
{
	if (!GetHUD())
	{
		return nullptr;
	}

	if (ATacticalHUD* THUD = Cast<ATacticalHUD>(GetHUD()))
	{
		return THUD->GetGameHUDWidget();
	}
	return nullptr;
}

void ATacticalPlayerController::StartRotatePreview()
{
	//UE_LOG(LogTemp, Log, TEXT("StartRot"));
	if (WeaponPreviewScene != nullptr)
	{
		WeaponPreviewScene->EnablePreviewRotation(true);
	}
}

void ATacticalPlayerController::StopRotatePreview()
{
	//UE_LOG(LogTemp, Log, TEXT("StopRot"));
	if (WeaponPreviewScene != nullptr)
	{
		WeaponPreviewScene->EnablePreviewRotation(false);
	}
}

void ATacticalPlayerController::RotatePreviewX(float Val)
{
	if (WeaponPreviewScene != nullptr)
	{
		WeaponPreviewScene->RotatePreviewX(Val);
	}
}

void ATacticalPlayerController::RotatePreviewY(float Val)
{
	if (WeaponPreviewScene != nullptr)
	{
		WeaponPreviewScene->RotatePreviewY(Val);
	}
}

void ATacticalPlayerController::XmlLoadTest(FString infile)
{
	UTacticalLoadoutManager* LM = GetMutableDefault<UTacticalLoadoutManager>();
	FTacticalLoadout LoadedLoadout = LM->LoadFromFile(infile);
	DumpLoadout(LoadedLoadout);
}

void ATacticalPlayerController::XmlWriteTest(FString infile)
{
	UTacticalLoadoutManager* LM = GetMutableDefault<UTacticalLoadoutManager>();
	LM->WriteToXmlFile( GetLoadout(), infile);
}

void ATacticalPlayerController::DumpLoadoutFiles()
{
	const UTacticalLoadoutManager* LM = GetDefault<UTacticalLoadoutManager>();

	TArray<FString> FoundFiles;
	LM->GetLoadoutUserFiles(FoundFiles);
	UE_LOG(LogTemp, Log, TEXT("Dumping Loadout files:"));
	for (FString& File : FoundFiles)
	{
		UE_LOG(LogTemp, Log, TEXT("    %s"), *File);
	}
}

void ATacticalPlayerController::DumpLoadout(const FTacticalLoadout& Loadout)
{
	UE_LOG(LogTemp, Log, TEXT("Loadout: %s"), *(Loadout.Name));
	UE_LOG(LogTemp, Log, TEXT("    MainWPN: %s"), Loadout.MainWeaponType ? *(Loadout.MainWeaponType->GetDefaultObject()->GetName()) : TEXT("None"));
	UE_LOG(LogTemp, Log, TEXT("        attachments: %s"), *(Loadout.MainWPNAttachmentString));
	UE_LOG(LogTemp, Log, TEXT("        ammo: %d"), Loadout.MainWPN_Ammo);
	UE_LOG(LogTemp, Log, TEXT("    Sidearm: %s"), Loadout.SidearmType ? *(Loadout.SidearmType->GetDefaultObject()->GetName()) : TEXT("None"));
	UE_LOG(LogTemp, Log, TEXT("        attachments: %s"), *(Loadout.SidearmAttachmentString));
	UE_LOG(LogTemp, Log, TEXT("        ammo: %d"), Loadout.Sidearm_Ammo);
	UE_LOG(LogTemp, Log, TEXT("    Gadget1: %s"), Loadout.Gadget1 ? *(Loadout.Gadget1->GetDefaultObject()->GetName()) : TEXT("None"));
	UE_LOG(LogTemp, Log, TEXT("        params: %s"), *(Loadout.Gadget1_Params));
	UE_LOG(LogTemp, Log, TEXT("    Gadget2: %s"), Loadout.Gadget2 ? *(Loadout.Gadget2->GetDefaultObject()->GetName()) : TEXT("None"));
	UE_LOG(LogTemp, Log, TEXT("        params: %s"), *(Loadout.Gadget2_Params));
}
