// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "BRSGameViewportClient.generated.h"



//struct FBRSGameLoadingScreenBrush : public FSlateDynamicImageBrush, public FGCObject
//{
//	FBRSGameLoadingScreenBrush(const FName InTextureName, const FVector2D& InImageSize)
//		: FSlateDynamicImageBrush(InTextureName, InImageSize)
//	{
//		ResourceObject = LoadObject<UObject>(NULL, *InTextureName.ToString());
//	}
//
//	virtual void AddReferencedObjects(FReferenceCollector& Collector)
//	{
//		if (ResourceObject)
//		{
//			Collector.AddReferencedObject(ResourceObject);
//		}
//	}
//};
//
//class  SBRSLoadingScreen : public SCompoundWidget
//{
//public:
//	SLATE_BEGIN_ARGS(SBRSLoadingScreen) {}
//	SLATE_END_ARGS()
//
//	void Construct(const FArguments& InArgs);
//
//private:
//	EVisibility GetLoadIndicatorVisibility() const
//	{
//		return EVisibility::Visible;
//	}
//
//	/** loading screen image brush */
//	TSharedPtr<FSlateDynamicImageBrush> LoadingScreenBrush;
//};

UCLASS(Within = Engine, transient, config = Engine)
class UBRSGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:
	UBRSGameViewportClient(const FObjectInitializer& OI);

	// start UGameViewportClient interface
	void NotifyPlayerAdded(int32 PlayerIndex, ULocalPlayer* AddedPlayer) override;
	void AddViewportWidgetContent(TSharedRef<class SWidget> ViewportContent, const int32 ZOrder = 0) override;
	void RemoveViewportWidgetContent(TSharedRef<class SWidget> ViewportContent) override;

	//void ShowDialog(TWeakObjectPtr<ULocalPlayer> PlayerOwner, EShooterDialogType::Type DialogType, const FText& Message, const FText& Confirm, const FText& Cancel, const FOnClicked& OnConfirm, const FOnClicked& OnCancel);
	//void HideDialog();

	void ShowLoadingScreen();
	void HideLoadingScreen();

	//bool IsShowingDialog() const { return DialogWidget.IsValid(); }

	//EShooterDialogType::Type GetDialogType() const;
	//TWeakObjectPtr<ULocalPlayer> GetDialogOwner() const;

	//TSharedPtr<SShooterConfirmationDialog> GetDialogWidget() { return DialogWidget; }

	//FTicker Funcs
	virtual void Tick(float DeltaSeconds) override;

#if WITH_EDITOR
	virtual void DrawTransition(class UCanvas* Canvas) override;
#endif //WITH_EDITOR
	// end UGameViewportClient interface

	virtual void LostFocus(class FViewport* inViewport) override;
	virtual void ReceivedFocus(class FViewport* inViewport) override;

protected:
	void HideExistingWidgets();
	void ShowExistingWidgets();

	/** List of viewport content that the viewport is tracking */
	TArray<TSharedRef<class SWidget>>				ViewportContentStack;

	TArray<TSharedRef<class SWidget>>				HiddenViewportContentStack;

	TSharedPtr<class SWidget>						OldFocusWidget;

	// /** Dialog widget to show temporary messages ("Controller disconnected", "Parental Controls don't allow you to play online", etc) */
	//TSharedPtr<SShooterConfirmationDialog>			DialogWidget;

	TSharedPtr<class SBRSLoadingScreen2>				LoadingScreenWidget;
};