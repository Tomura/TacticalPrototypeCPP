// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GenericTeamAgentInterface.h"
#include "TacticalControllerInterface.h"
#include "BRSPlayerController_Base.h"
#include "TacticalLoadout.h"
#include "TacticalPlayerController.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPawnDiedSignature);

/**
 * 
 */
UCLASS()
class TACTICALFPS_API ATacticalPlayerController : public ABRSPlayerController_Base, public IGenericTeamAgentInterface, public ITacticalControllerInterface
{
	GENERATED_BODY()
public:
	ATacticalPlayerController(const FObjectInitializer& OI);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void TickLocalPlayerOnly(float DeltaTime);
	virtual void SetupInputComponent() override;

	virtual void UnPossess() override;

	virtual void FOV(float NewFOV) override;


	virtual bool IsInMenu() const override;
	virtual bool IsInChat() const;

	virtual void AddClientTravelParams(FString& URL) override;

	/** Assigns Team Agent to given TeamID */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	/** Retrieve team identifier in form of FGenericTeamId */
	virtual FGenericTeamId GetGenericTeamId() const override;

	/** Retrieved owner attitude toward given Other object */
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;


	// TacticalControllerInterface
	virtual void OnPawnDied_Implementation(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser) override;
	virtual void OnPawnTakeDamage_Implementation(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser) override;

	virtual void Reset() override;
	
	UFUNCTION(reliable, client)
	void ClientResetLevel();
	virtual void ClientResetLevel_Implementation();


	virtual bool IsAlliedTo(APlayerState* OtherPlayer) const;

	UFUNCTION(exec, BlueprintCallable, Category = "Player Camera")
	virtual void SetDefaultFOV(float NewFOV, bool bSave = true);

	UFUNCTION(BlueprintCallable, Category = "Player Camera")
	virtual float GetDefaultFOV() const;

	float GetPredictionTime() const;


	void ToggleMenu();
	UFUNCTION(Category = "HUD", BlueprintCallable)
	virtual void ShowPauseMenu(bool bShow);


	UFUNCTION(BlueprintCallable, Category = "Player")
	virtual void NotifyReadyToPlay(bool bReady);

	UFUNCTION(Reliable, Client)
	void ClientOpenPreRoundMenu();
	virtual void ClientOpenPreRoundMenu_Implementation();

	//UFUNCTION(Exec)
	//void SetReady(bool bReady);


	UFUNCTION(Reliable, Server, WithValidation)
	void ClientForceReady();
	virtual void ClientForceReady_Implementation();
	bool ClientForceReady_Validate() { return true; }

	UFUNCTION(Reliable, Server, WithValidation, Category = "Tactical Player Controller")
	void ServerSetReady(bool bReady);
	virtual void ServerSetReady_Implementation(bool bReady);
	bool ServerSetReady_Validate(bool bReady){ return true; }

	UFUNCTION(Reliable, Server, WithValidation, Category = "TacticalPlayerController")
	void ServerSetLoadoutAndReady(struct FTacticalLoadout ConfirmedLoadout);
	void ServerSetLoadoutAndReady_Implementation(struct FTacticalLoadout ConfirmedLoadout);
	bool ServerSetLoadoutAndReady_Validate(struct FTacticalLoadout ConfirmedLoadout) { return true; }


	UFUNCTION(BlueprintCallable, Category = "HUD")
		class UTacticalPauseMenu* GetPauseMenuWidget() const;
	UFUNCTION(BlueprintCallable, Category = "HUD")
		class UUserWidget* GetGameHUDWidget() const;

	UFUNCTION(BlueprintCallable, Category = "Loadout")
	FTacticalLoadout GetLoadout() const{ return SpawningLoadout; }

	UFUNCTION(BlueprintCallable, Category = "Loadout")
	TSubclassOf<ATacticalPrimaryWeapon> GetMainWeapon() const { return SpawningLoadout.MainWeaponType; }

	UFUNCTION(BlueprintCallable, Category = "Loadout")
	TSubclassOf<ATacticalSidearm> GetSidearm() const{ return SpawningLoadout.SidearmType; }

	UFUNCTION(BlueprintCallable, Category = "Loadout")
	void LocalSetLoadout(const FTacticalLoadout& NewLoadout, bool bSave);

	UFUNCTION(BlueprintCallable, Category = "Loadout", Reliable, Client)
	void ClientSetLoadout(FTacticalLoadout NewLoadout);
	void ClientSetLoadout_Implementation(FTacticalLoadout NewLoadout);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSetLoadout(FTacticalLoadout NewLoadout);
	void ServerSetLoadout_Implementation(FTacticalLoadout NewLoadout);
	bool ServerSetLoadout_Validate(FTacticalLoadout NewLoadout) { return true; }

	UFUNCTION(BlueprintCallable, Category = "Loadout")
	void LocalSetLoadoutAttachment(class UTacticalWeaponAttachmentPoint* AttachPoint, int32 AttachmentIdx, EWeaponClass WeaponClass, bool bSave);
	void LocalSetLoadoutAttachment(const FString& AttachPoint, int32 AttachmentIdx, EWeaponClass WeaponClass, bool bSave);


	UFUNCTION(BlueprintCallable, Category = "Loadout")
	void LocalSetLoadoutWeapon(TSubclassOf<class ATacticalWeapon> WeaponType, bool bSave);

	UFUNCTION(BlueprintCallable, Category = "Loadout")
	void LocalSetLoadoutAmmo(int32 AmmoIdx, EWeaponClass WeaponClass, bool bSave);

	UFUNCTION(BlueprintCallable, Category = "Loadout")
	void LocalSetLoadoutGadget(int32 GadgetNum, TSubclassOf<class ATacticalInventory> ItemType, bool bSave);

	UFUNCTION()
	void OpenChat() { OpenChatWidget(false); }
	UFUNCTION()
	void OpenTeamChat(){ OpenChatWidget(true); };

	UFUNCTION(Category = "Chat", BlueprintNativeEvent)
	void OpenChatWidget(bool bTeamChat = false);
	void OpenChatWidget_Implementation(bool bTeamChat = false);


	bool ToggleAim() const;
	bool ToggleCrouch() const;

	UFUNCTION(Category = "Tactical Player", BlueprintCallable)
	class UPostProcessComponent* GetPostProcessComponent() const { return PlayerPostProcessComp; }



	UPROPERTY(BlueprintAssignable, Category = "Health Component")
	FOnPawnDiedSignature OnPawnDiedDelegate;


	UFUNCTION(Category = "Tactical Player", BlueprintCallable)
	class ATacticalWeaponPreviewScene* FocusWeaponPreview();

	UFUNCTION(Category = "Tactical Player", BlueprintCallable)
		void UnFocusWeaponPreview();

	//UFUNCTION(Category = "Tactical Player", BlueprintCallable, reliable, Client)
	//class ATacticalWeaponPreviewScene* FocusWeaponPreview();
	//virtual class ATacticalWeaponPreviewScene* FocusWeaponPreview_Implementation();

	bool IsUsingFreecam() const;

	virtual void LeftMouseButtonDown();

protected:
	FGenericTeamId TeamID;

	bool bInMenu;

	/** Maximum allowed Ping for Rewind or other Prediction methods for ping compensation*/
	float MaxPredictionPing;
	/** Fudge factor for prediction/ping compensation */
	float PredictionFudgeFactor;

	/** Widget Class for Main Gameplay HUD */
	UPROPERTY(Category = "Classes | HUD", EditDefaultsOnly)
	TSubclassOf<class UUserWidget> GameHUDClass;
	/** Widget Class for Pause Menu */
	UPROPERTY(Category = "Classes | HUD", EditDefaultsOnly)
	TSubclassOf<class UTacticalPauseMenu> PauseMenuClass;

	/** Timer for the Pause Menu open animation (acts as cooldown for the Pause Menu Button) */
	FTimerHandle MenuTimer;



	UPROPERTY(Category = "Loadout", EditAnywhere, GlobalConfig)
	struct FTacticalLoadout SpawningLoadout;


	void StartRotatePreview();
	void StopRotatePreview();
	void RotatePreviewX(float Val);
	void RotatePreviewY(float Val);

	class ATacticalWeaponPreviewScene* WeaponPreviewScene;
	bool bRotateWeaponPreview;


	//Debug
	UFUNCTION(Exec)
	void XmlLoadTest(FString infile);
	UFUNCTION(Exec)
	void XmlWriteTest(FString infile);

	UFUNCTION(Exec)
	void DumpLoadoutFiles();

	void DumpLoadout(const FTacticalLoadout& Loadout);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UPostProcessComponent* PlayerPostProcessComp;


	///** Widget for the main Game HUD */
	//UPROPERTY(Transient)
	//class UUserWidget* GameHUDWidget;
	///** Widget for the Pause Menu */
	//UPROPERTY(Transient)
	//class UTacticalPauseMenu* PauseMenuWidget;

	///** Widget to show in-between rounds */
	//UPROPERTY(Transient)
	//class UUserWidget* PreRoundWidget;
};
