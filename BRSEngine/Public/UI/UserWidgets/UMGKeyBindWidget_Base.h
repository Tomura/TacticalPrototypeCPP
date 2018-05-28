// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "UMGKeyBindWidget_Base.generated.h"

USTRUCT(BlueprintInternalUseOnly)
struct BRSENGINE_API FBRSKeyBindData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Binding")
		FString Mapping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Binding")
		float Scale;

	FBRSKeyBindData() : Mapping(FString()), Scale(0.f)
	{}
};


UENUM()
namespace EWaitingForKeyState
{
	enum Type
	{
		WAIT_None,
		WAIT_Key,
		WAIT_Alt
	};
}

/**
 * 
 */
UCLASS(abstract)
class BRSENGINE_API UUMGKeyBindWidget_Base : public UUserWidget
{
	GENERATED_BODY()

public:
	UUMGKeyBindWidget_Base(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Key Binding")
	FText DisplayText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Key Binding")
	TArray<FBRSKeyBindData> Binds;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Key Binding")
	FKey DefaultKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Key Binding")
	FKey DefaultAltKey;


	TSharedPtr<FKey> Key;

	TSharedPtr<FKey> AltKey;




	TArray<FInputActionKeyMapping> ActionMappings;
	TArray<FInputAxisKeyMapping> AxisMappings;

	UUMGKeyBindWidget_Base* AddMapping(const FString& Mapping, float Scale = 0.0f);
	UUMGKeyBindWidget_Base* AddDefaults(FKey InDefaultKey, FKey InDefaultAltKey = FKey())
	{
		DefaultKey = InDefaultKey;
		DefaultAltKey = InDefaultAltKey;
		return this;
	}

	UFUNCTION(BlueprintCallable, Category = "Key Binding")
	void WriteBind();
	

	void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Key Binding")
	void GetKeys(FKey& OutKey, FKey& OutAltKey) const;

	UFUNCTION(BlueprintCallable, Category = "Key Binding")
	void SetKey(FKey InKey, bool bAltKey = false);

	//virtual FEventReply OnKeyDown_Implementation(FGeometry MyGeometry, FKeyboardEvent InKeyboardEvent) override;
	//virtual FEventReply OnMouseButtonDown_Implementation(struct FGeometry MyGeometry, const struct FPointerEvent & MouseEvent) override;
	//virtual FEventReply OnMouseWheel_Implementation(FGeometry MyGeometry, const FPointerEvent& MouseEvent) override;

	bool bCanReset;

	UPROPERTY(BlueprintReadOnly, Category = "Key Binding")
	TEnumAsByte<EWaitingForKeyState::Type> WaitingForKey;

	UFUNCTION(BlueprintCallable, Category = "Key Binding")
	void WaitForKey(UButton* TriggeringWidget, bool bAltKey = false);

	UFUNCTION(BlueprintCallable, Category = "Key Binding")
	void BindWidgetToKey(class UBRSKeyBind* Widget, bool bIsAlt = false);

	void OnKeyRebound();

	void ResetToDefaults();
};
