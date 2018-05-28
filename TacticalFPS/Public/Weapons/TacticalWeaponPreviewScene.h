// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/Actor.h"
#include "TacticalTypes.h"
#include "TacticalWeaponPreviewScene.generated.h"

UCLASS(abstract)
class TACTICALFPS_API ATacticalWeaponPreviewScene : public AActor
{
	GENERATED_BODY()


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* PreviewCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* WeaponScene;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* WeaponScene_Pitch;

public:
	// Sets default values for this actor's properties
	ATacticalWeaponPreviewScene();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Destroyed() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;


	class UCameraComponent* GetPreviewCamera() const { return PreviewCamera; }
	class USceneComponent* GetWeaponScene() const { return WeaponScene; }

	UFUNCTION(Category = "Weapon Preview", BlueprintCallable)
	void SetWeapon(TSubclassOf<class ATacticalWeapon> WeaponClass, APlayerController* PC, EWeaponClass WPNType);


	UFUNCTION(Category = "Weapon Preview", BlueprintCallable)
	void SetPreviewWeapon(EWeaponClass WeaponClass, APlayerController* PC);


	UFUNCTION(Category = "Weapon Preview", BlueprintCallable)
		class ATacticalWeapon* GetWeapon() const { return Weapon; }
	UFUNCTION(Category = "Weapon Preview", BlueprintCallable)
		class ATacticalWeapon* GetSidearm() const { return Sidearm; }
	UFUNCTION(Category = "Weapon Preview", BlueprintCallable)
		class ATacticalWeapon* GetPreviewWeapon() const { return PreviewWeapon; }
	
	UFUNCTION(Category = "Weapon Preview", BlueprintNativeEvent)
	USkeletalMeshComponent* GetWeaponMesh() const;
	USkeletalMeshComponent* GetWeaponMesh_Implementation() const;



	void AddWeaponAttachment(const FString& AttachmentPoint, int32 AttachmentIdx, EWeaponClass WPNClass);

	void SetAmmoPreview(TSubclassOf<class UTacticalAmmoType> AmmoClass);

	UFUNCTION(Category = "Weapon Preview", BlueprintCallable)
	void SetAmmoPreview(int32 AmmoIdx);

	UFUNCTION(Category = "Weapon Preview", BlueprintNativeEvent)
	void SetAmmoPreviewMeshes(UStaticMesh* BoxMesh, UStaticMesh* BulletMesh);
	virtual void SetAmmoPreviewMeshes_Implementation(UStaticMesh* BoxMesh, UStaticMesh* BulletMesh) {}

	UFUNCTION(Category = "Weapon Preview", BlueprintCallable)
	void SetFromPlayer(class ATacticalPlayerController* Controller);


	virtual void OnFocusPreview(class ATacticalPlayerController* PC);
	virtual void OnUnfocusPreview(class ATacticalPlayerController* PC);

	virtual void UpdateAttachmentWidgets(class APlayerController* PC);
	virtual void RemoveAllAttachmentWidgets();

	UPROPERTY(Category = "Weapon Preview", EditDefaultsOnly)
		TSubclassOf<class UTacticalWPNAttachmentSelection> AttachmentWidgetClass;
	UPROPERTY(Category = "Weapon Preview", EditDefaultsOnly)
		TSubclassOf<class UTacticalWPNAmmoSelection> AmmoWidgetClass;
	UPROPERTY(Category = "Weapon Preview", BlueprintReadOnly)
	TArray<class UTacticalWPNAttachmentSelection*> AttachmentWidgets;
	class UTacticalWPNAmmoSelection* AmmoWidget;

	UFUNCTION(Category = "Weapon Preview", BlueprintImplementableEvent, BlueprintCallable)
	class UPrimitiveComponent* GetAmmoMesh() const;

	void EnablePreviewRotation(bool bEnabled);
	void RotatePreviewX(float Delta);
	void RotatePreviewY(float Delta);

	virtual void OnClickedOutside();

	class ATacticalWeapon* SpawnPreviewWeapon(TSubclassOf<ATacticalWeapon> WeaponClass);
	void SetupAmmoPreview(ATacticalWeapon* PreviewWeapon, APlayerController* PC);

	

public:

	class ATacticalWeapon* PreviewWeapon;

	UPROPERTY()
	class ATacticalWeapon* Weapon;

	UPROPERTY()
	class ATacticalWeapon* Sidearm;


	UPROPERTY(Category = "Weapon Preview", EditDefaultsOnly)
	float RotationRate;
	bool bRotatePreviewMesh;

};