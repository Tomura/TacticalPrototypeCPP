// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "AI/Navigation/NavLinkDefinition.h"
#include "AI/Navigation/NavLinkHostInterface.h"
#include "AI/Navigation/NavRelevantInterface.h"
#include "GameFramework/Actor.h"
#include "TacticalVaultMarker.generated.h"

UCLASS(Blueprintable)
class ATacticalVaultMarker : public AActor, public INavLinkHostInterface, public INavRelevantInterface
{
	GENERATED_BODY()
public:
	ATacticalVaultMarker();

	// reference to AI/Navigation/NavLinkProxy.h for more
	/** Navigation links (point to point) added to navigation data */
	UPROPERTY(EditAnywhere, Category = SimpleLink)
	TArray<FNavigationLink> PointLinks;

	/** Navigation links (segment to segment) added to navigation data
	*	@todo hidden from use until we fix segment links. Not really working now*/
	//UPROPERTY()
	//	TArray<FNavigationSegmentLink> SegmentLinks;

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void UpdateBoxes();

	FORCEINLINE bool ShouldClimb() const { return bClimb; }

	// BEGIN INavRelevantInterface
	virtual void GetNavigationData(FNavigationRelevantData& Data) const override;
	virtual FBox GetNavigationBounds() const override;
	virtual bool IsNavigationRelevant() const override;
	// END INavRelevantInterface

	// BEGIN INavLinkHostInterface
	virtual bool GetNavigationLinksClasses(TArray<TSubclassOf<UNavLinkDefinition> >& OutClasses) const override;
	virtual bool GetNavigationLinksArray(TArray<FNavigationLink>& OutLink, TArray<FNavigationSegmentLink>& OutSegments) const override;
	// END INavLinkHostInterface

	virtual FBox GetComponentsBoundingBox(bool bNonColliding = false) const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	//virtual void PostInitializeComponents() override;
	//virtual void PostLoad() override;

protected:
	UPROPERTY(Category = Dimensions, EditAnywhere)
	float Length;
	UPROPERTY(Category = Dimensions, EditAnywhere)
	float Thickness;	
	UPROPERTY(Category = Dimensions, EditAnywhere)
	float NavLinkSpacing;

	UPROPERTY(Category = Dimensions, EditAnywhere, meta=(EditCond=bClimb))
	TEnumAsByte<ENavLinkDirection::Type> Direction;

	float AddedLength;
	float AddedThickness;

	UPROPERTY(Category = "Vault Marker", EditAnywhere)
	uint8 bClimb : 1;

private:
	UPROPERTY()
	UBoxComponent* CollisionComp;

	UPROPERTY()
	UChildActorComponent* NavLinkComp;

#if WITH_EDITORONLY_DATA
protected:
	UTexture2D* VaultSprite;
	UTexture2D* ClimbSprite;

private:
	UPROPERTY()
	UBillboardComponent* Sprite;
	UPROPERTY()
	UArrowComponent* ForwardArrow;
	UPROPERTY()
	UArrowComponent* BackwardArrow;
	UPROPERTY()
	UBoxComponent* PreviewComp;
#endif

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY()
	class UNavLinkRenderingComponent* EdRenderComp;
public:

	/** Returns EdRenderComp subobject **/
	UNavLinkRenderingComponent* GetEdRenderComp() const;
#endif
};