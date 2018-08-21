// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "TacticalWeaponAttachment.generated.h"


UCLASS(abstract)
class TACTICALFPS_API ATacticalWeaponAttachment : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* Root;

public:
	ATacticalWeaponAttachment(const FObjectInitializer& OI);
	virtual void BeginPlay() override;

	virtual void OnCharacterOwnerDied();
	virtual void OnWeaponRemovedFromOwner();

	virtual void OnFirstPerson();
	virtual void OnThirdPerson();

	UFUNCTION(Category = "Weapon Attachment", BlueprintImplementableEvent, meta = (DisplayName = "On First Person"))
		void BP_OnFirstPerson();
	UFUNCTION(Category = "Weapon Attachment", BlueprintImplementableEvent, meta = (DisplayName = "On Third Person"))
		void BP_OnThirdPerson();

	void SetupPreviewLighting();

	UFUNCTION(Category = "Weapon Attachment", BlueprintNativeEvent)
	float ModifySpread(float OriginalSpread);
	virtual float ModifySpread_Implementation(float OriginalSpread) { return OriginalSpread; }
	UFUNCTION(Category = "Weapon Attachment", BlueprintNativeEvent)
	float ModifyRecoil(float OriginalRecoil);
	virtual float ModifyRecoil_Implementation(float OriginalRecoil) { return OriginalRecoil; }
	UFUNCTION(Category = "Weapon Attachment", BlueprintNativeEvent)
	float ModifyDamage(float OriginalDamage);
	virtual float ModifyDamage_Implementation(float OriginalDamage) { return OriginalDamage; }
	UFUNCTION(Category = "Weapon Attachment", BlueprintNativeEvent)
	float ModifyRange(float OriginalRange);
	virtual float ModifyRange_Implementation(float OriginalRange) { return OriginalRange; }
	UFUNCTION(Category = "Weapon Attachment", BlueprintNativeEvent)
	float ModifyAimSpeed(float OriginalSpeed);
	virtual float ModifyAimSpeed_Implementation(float OriginalSpeed) { return OriginalSpeed; }
	

	UFUNCTION(Category = "Weapon Attachment", BlueprintCallable)
	const FText& GetAttachmentName() const { return AttachmentName; }


	UFUNCTION(Category = "Weapon Attachment", BlueprintCallable)
		class ATacticalCharacter* GetCharacterOwner() const;
	UFUNCTION(Category = "Weapon Attachment", BlueprintCallable)
	class ATacticalWeapon* GetWeaponOwner() const;

	void OnStartAiming();
	void OnStopAiming();

	UFUNCTION(Category = "Weapon Attachment", BlueprintImplementableEvent, meta = (DisplayName = "On Start Aiming"))
		void BP_OnStartAiming();
	UFUNCTION(Category = "Weapon Attachment", BlueprintImplementableEvent, meta = (DisplayName = "On Stop Aiming"))
		void BP_OnStopAiming();


	UFUNCTION(Category = "Weapon Attachment", BlueprintNativeEvent)
	void OnTurnOn(bool bIsOn);
	virtual void OnTurnOn_Implementation(bool bIsOn) {}

	UFUNCTION(Category = "Weapon Attachment", BlueprintNativeEvent)
	void OnApplyOptions(const FString& Options);
	virtual void OnApplyOptions_Implementation(const FString& Options) {}

protected:

	UPROPERTY(Category = Attachment, EditDefaultsOnly, BlueprintReadOnly)
	FText AttachmentName;


	//UPROPERTY(ReplicatedUsing = OnRepWeaponOwner)
	class ATacticalWeapon* WeaponOwner;
	class UTacticalWeaponAttachmentPoint* OwnerAttachmentPoint;
};


UCLASS(abstract)
class TACTICALFPS_API ATacticalWeaponAttachment_Muzzle :public ATacticalWeaponAttachment
{
	GENERATED_BODY()
public:
	ATacticalWeaponAttachment_Muzzle(const FObjectInitializer& OI);

	FORCEINLINE float GetSpreadModifier() const { return SpreadModifier; }
	FORCEINLINE float GetRecoilModifier() const { return RecoilModifier; }
	FORCEINLINE float GetSpreadRegenerationModifier() const { return SpreadRegenModifier; }
	FORCEINLINE float GetNoiseModifier() const { return NoiseModifier; }
	FORCEINLINE bool ShouldSkipLocalSimulation() const { return bSkipLocalSimulateFire; }

	UFUNCTION(Category = "Attachment", BlueprintNativeEvent)
		void LocalSimulateFire();
	virtual void LocalSimulateFire_Implementation() {}

	bool IsSilencer() { return bSilenced; }

	UFUNCTION(Category = "Attachment", BlueprintNativeEvent)
	UParticleSystemComponent* GetMuzzleFlashPSC() const;
	virtual UParticleSystemComponent* GetMuzzleFlashPSC_Implementation() const { return nullptr; }

protected:
	UPROPERTY(Category = Attachment, EditDefaultsOnly)
		bool bSilenced;
	UPROPERTY(Category = Attachment, EditDefaultsOnly)
		float SpreadModifier;
	UPROPERTY(Category = Attachment, EditDefaultsOnly)
		float RecoilModifier;
	UPROPERTY(Category = Attachment, EditDefaultsOnly)
		float SpreadRegenModifier;

	UPROPERTY(Category = Attachment, EditDefaultsOnly)
		float NoiseModifier;

	UPROPERTY(Category = Attachment, EditDefaultsOnly)
		uint8 bSkipLocalSimulateFire : 1;
};

UCLASS(abstract)
class TACTICALFPS_API ATacticalWeaponAttachment_Sight : public ATacticalWeaponAttachment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* SightAimPoint;

public:
	ATacticalWeaponAttachment_Sight(const FObjectInitializer& OI);


	virtual void OnCharacterOwnerDied() override;
	virtual void OnWeaponRemovedFromOwner() override;

	virtual void OnFirstPerson() override;
	virtual void OnThirdPerson() override;

	UFUNCTION(Category = "Weapon Attachment|Sight", BlueprintCallable)
		FORCEINLINE bool IsFirstPerson() const;


	FORCEINLINE FVector GetAimOffset() const { return AimOffset; }
	FORCEINLINE float GetZoomModifier() const { return AimZoomModifier; }

	UFUNCTION(Category = "Weapon Attachment|Sight", BlueprintNativeEvent)
	FVector ModifyAimOffset(const FVector& DefaultOffset) const;
	virtual FVector ModifyAimOffset_Implementation(const FVector& DefaultOffset) const;

	UFUNCTION(Category = "Weapon Attachment|Sight", BlueprintCallable)
	float GetRecommendedRenderTargetResolution() const;
	UFUNCTION(Category = "Weapon Attachment|Sight", BlueprintCallable)
	float GetRecommendedFieldOfView() const;

	UFUNCTION(Category = "Weapon Attachment|Sight", BlueprintCallable, BlueprintNativeEvent)
	class USceneCaptureComponent2D* GetSceneCapture() const;
	virtual class USceneCaptureComponent2D* GetSceneCapture_Implementation() const { return nullptr; }

	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(Category = Sight, EditDefaultsOnly)
		uint32 bOverrideAimOffset : 1;

	UPROPERTY(Category = Sight, EditDefaultsOnly, BlueprintReadOnly)
		FVector AimOffset;

	UPROPERTY(Category = Sight, EditDefaultsOnly, BlueprintReadOnly)
		float AimZoomModifier;

	UPROPERTY(Category = Sight, EditDefaultsOnly)
		float SightScreenPercent;

	UPROPERTY(Category = "Depth of Field", EditDefaultsOnly)
		bool bModifyDepthOfField;
	UPROPERTY(Category = "Depth of Field", EditDefaultsOnly, meta = (EditCondition = "bModifyDepthOfField"))
		float AimedNearBlurSizeMod;
	UPROPERTY(Category = "Depth of Field", EditDefaultsOnly, meta = (EditCondition = "bModifyDepthOfField"))
		float AimedFarBlurSizeMod;
	UPROPERTY(Category = "Depth of Field", EditDefaultsOnly, meta = (EditCondition = "bModifyDepthOfField"))
		float AimedFocusDistanceMod;
	UPROPERTY(Category = "Depth of Field", EditDefaultsOnly, meta = (EditCondition = "bModifyDepthOfField"))
		float AimedFStop;

};



UCLASS(abstract)
class TACTICALFPS_API ATacticalWeaponAttachment_Special : public ATacticalWeaponAttachment
{
	GENERATED_BODY()

	friend class ATacticalWeapon;

public:
	ATacticalWeaponAttachment_Special(const FObjectInitializer& OI);

	FORCEINLINE float GetSpreadModifier() const { return SpreadModifier; }
	FORCEINLINE float GetRecoilModifier() const { return RecoilModifier; }
	FORCEINLINE float GetSpreadRegenerationModifier() const { return SpreadRegenModifier; }

	UFUNCTION(BlueprintNativeEvent, Category = Attachment)
	bool GetLeftHandTransform(FTransform& OutTransform);
	virtual bool GetLeftHandTransform_Implementation(FTransform& OutTransform);


	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animation")
	UAnimSequence* FPPLeftHandAnimation;

	UFUNCTION(BlueprintNativeEvent, Category = Attachment)
	UAnimSequence* GetFPHandAnimation_Left() const;
	virtual UAnimSequence* GetFPHandAnimation_Left_Implementation() const { return FPPLeftHandAnimation; }

	UFUNCTION(BlueprintNativeEvent, Category = Attachment)
	UAnimSequence* GetFPHandAnimation_Right() const;
	virtual UAnimSequence* GetFPHandAnimation_Right_Implementation() const { return nullptr; }

protected:



protected:
	UPROPERTY(Category = Attachment, EditDefaultsOnly)
		float SpreadModifier;
	UPROPERTY(Category = Attachment, EditDefaultsOnly)
		float RecoilModifier;
	UPROPERTY(Category = Attachment, EditDefaultsOnly)
		float SpreadRegenModifier;
};