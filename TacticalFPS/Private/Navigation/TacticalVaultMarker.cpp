// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "ObjectEditorUtils.h"
#include "NavigationModifier.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/Navigation/NavLinkRenderingComponent.h"
#include "AI/NavigationOctree.h"
#include "NavigationSystemHelpers.h"
#include "TacticalVaultMarker.h"
#include "NavArea_Vault.h"
#include "AI/Navigation/NavAreas/NavArea_Default.h"

ATacticalVaultMarker::ATacticalVaultMarker()
	: Super()
{
#if WITH_EDITORONLY_DATA
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> VaultSpriteTextureObject;
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> ClimbSpriteTextureObject;
		FConstructorStatics()
			: VaultSpriteTextureObject(TEXT("/Game/Icons/T_VaultMarker"))
			, ClimbSpriteTextureObject(TEXT("/Game/Icons/T_ClimbMarker"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;
#endif

	Length = 100.f;
	Thickness = 20.f;

	AddedLength = -60.f;
	AddedThickness = 70.f;

	bClimb = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DummyRoot"));

	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	CollisionComp->SetupAttachment(RootComponent);
	CollisionComp->SetRelativeLocation(FVector::ZeroVector);
	CollisionComp->SetRelativeRotation(FRotator::ZeroRotator);
	CollisionComp->SetBoxExtent(FVector((Thickness * 0.5f) + AddedThickness, (Length * 0.5f) + AddedLength, 10.f));
	CollisionComp->SetCollisionProfileName(TEXT("IgnoreExceptPawn"));
	CollisionComp->ShapeColor = FColor::Magenta;


#if WITH_EDITORONLY_DATA
	PreviewComp = CreateDefaultSubobject<UBoxComponent>(TEXT("PreviewBox"));
	PreviewComp->SetupAttachment(RootComponent);
	PreviewComp->SetRelativeLocation(FVector::ZeroVector);
	PreviewComp->SetRelativeRotation(FRotator::ZeroRotator);
	PreviewComp->SetBoxExtent(FVector(Thickness * 0.5f, Length * 0.5f, 10.f));
	PreviewComp->SetCollisionProfileName(TEXT("NoCollision"));
	PreviewComp->ShapeColor = FColor::Purple;

	ForwardArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardArrow"));
	ForwardArrow->SetupAttachment(RootComponent);
	ForwardArrow->SetArrowColor(FColor::Red);
	ForwardArrow->SetRelativeRotation(FRotator::ZeroRotator);
	ForwardArrow->SetRelativeLocation(FVector::ZeroVector);
	ForwardArrow->bHiddenInGame = true;

	BackwardArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("BackwardArrow"));
	BackwardArrow->SetupAttachment(RootComponent);
	BackwardArrow->SetArrowColor(FColor::Magenta);
	BackwardArrow->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	BackwardArrow->SetRelativeLocation(FVector::ZeroVector);
	BackwardArrow->bHiddenInGame = true;

	VaultSprite = ConstructorStatics.VaultSpriteTextureObject.Get();
	ClimbSprite = ConstructorStatics.ClimbSpriteTextureObject.Get();


	Sprite = nullptr;
	Sprite = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (Sprite)
	{
		Sprite->SetupAttachment(RootComponent);
		Sprite->SetSprite(VaultSprite);
	}


	EdRenderComp = CreateDefaultSubobject<UNavLinkRenderingComponent>(TEXT("EdRenderComp"));
	EdRenderComp->PostPhysicsComponentTick.bCanEverTick = false;
	EdRenderComp->SetupAttachment(RootComponent);

#endif

	NavLinkSpacing = 100.f;
}

void ATacticalVaultMarker::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateBoxes();
}

void ATacticalVaultMarker::BeginPlay()
{
	Super::BeginPlay();
	UpdateBoxes();
}

void ATacticalVaultMarker::UpdateBoxes()
{
	if (CollisionComp)
	{
		CollisionComp->SetRelativeLocation(FVector::ZeroVector);
		CollisionComp->SetRelativeRotation(FRotator::ZeroRotator);
		CollisionComp->SetBoxExtent(FVector((Thickness * 0.5f) + AddedThickness, (Length * 0.5f) + AddedLength, 10.f));
	}

	const float TraceZDist = -100.f;
	const float PointXSpace = 100.f;
	int32 NeededPointLinks = Length / NavLinkSpacing - 1;
	PointLinks.SetNum(NeededPointLinks * (bClimb ? 2.f : 1.f));
	const float ActualPointSpace = Length / (NeededPointLinks+1);
	for (int32 i = 0; i < NeededPointLinks; i++)
	{
		if(PointLinks.IsValidIndex(i))
		{
			FNavigationLink& Link = PointLinks[i];
			Link.SetAreaClass(UNavArea_Vault::StaticClass());
			float ypos = (i+1) * ActualPointSpace - 0.5f * Length;
			Link.Left = FVector(-PointXSpace, ypos, TraceZDist);
			Link.Right = FVector(PointXSpace, ypos, TraceZDist);
			Link.Direction = ENavLinkDirection::BothWays;
			if (bClimb)
			{
				Link.Direction = Direction;
				if (Direction == ENavLinkDirection::LeftToRight)
				{
					Link.Right.Z = 0.f;
				}
				else if(Direction == ENavLinkDirection::RightToLeft)
				{
					Link.Left.Z = 0.f;
				}
			}
			else
			{
				Link.Direction = Direction;
			}
		}
	}
	// if climbing marker then we need to tell that you can jump off.
	if (bClimb)
	{
		for (int32 i = 0; i < NeededPointLinks; i++)
		{
			int32 pointidx = i + NeededPointLinks;
			if (PointLinks.IsValidIndex(pointidx))
			{
				FNavigationLink& Link = PointLinks[pointidx];
				Link.SetAreaClass(UNavArea_Default::StaticClass());
				float ypos = (i + 1) * ActualPointSpace - 0.5f * Length;
				Link.Left = FVector(-PointXSpace, ypos, TraceZDist);
				Link.Right = FVector(PointXSpace, ypos, TraceZDist);
				Link.Direction = ENavLinkDirection::BothWays;
				if (bClimb)
				{
					
					if (Direction == ENavLinkDirection::LeftToRight)
					{
						Link.Direction = ENavLinkDirection::RightToLeft;
						Link.Right.Z = 0.f;
					}
					else if (Direction == ENavLinkDirection::RightToLeft)
					{
						Link.Direction = ENavLinkDirection::LeftToRight;
						Link.Left.Z = 0.f;
					}
				}
			}
		}
	}

#if WITH_EDITORONLY_DATA
	if (Sprite)
	{
		Sprite->SetSprite(bClimb ? ClimbSprite : VaultSprite);
	}

	if (PreviewComp)
	{
		PreviewComp->SetRelativeLocation(FVector::ZeroVector);
		PreviewComp->SetRelativeRotation(FRotator::ZeroRotator);
		PreviewComp->SetBoxExtent(FVector(Thickness * 0.5f, Length * 0.5f, 10.f));
	}
#endif

	// maybe only WITH_EDITOR
	UNavigationSystem* NavSys = UNavigationSystem::GetCurrent(GetWorld());
	if (NavSys)
	{
		NavSys->UpdateActorInNavOctree(*this);
	}
}


// NavlinkHostInterface
void ATacticalVaultMarker::GetNavigationData(FNavigationRelevantData& Data) const
{
	NavigationHelper::ProcessNavLinkAndAppend(&Data.Modifiers, this, PointLinks);
	//NavigationHelper::ProcessNavLinkSegmentAndAppend(&Data.Modifiers, this, SegmentLinks);
}

FBox ATacticalVaultMarker::GetNavigationBounds() const
{
	return GetComponentsBoundingBox();
}

bool ATacticalVaultMarker::IsNavigationRelevant() const
{
	return (PointLinks.Num() > 0); // || (SegmentLinks.Num() > 0) || bSmartLinkIsRelevant;
}

bool ATacticalVaultMarker::GetNavigationLinksClasses(TArray<TSubclassOf<UNavLinkDefinition> >& OutClasses) const
{
	return false;
}

bool ATacticalVaultMarker::GetNavigationLinksArray(TArray<FNavigationLink>& OutLink, TArray<FNavigationSegmentLink>& OutSegments) const
{
	OutLink.Append(PointLinks);
	//OutSegments.Append(SegmentLinks);

	return (PointLinks.Num() > 0); // || (SegmentLinks.Num() > 0);
}

FBox ATacticalVaultMarker::GetComponentsBoundingBox(bool bNonColliding /*= false*/) const
{
	FBox LinksBB(ForceInitToZero);
	LinksBB += FVector(0, 0, -10);
	LinksBB += FVector(0, 0, 10);

	for (int32 i = 0; i < PointLinks.Num(); ++i)
	{
		const FNavigationLink& Link = PointLinks[i];
		LinksBB += Link.Left;
		LinksBB += Link.Right;
	}

	//for (int32 i = 0; i < SegmentLinks.Num(); ++i)
	//{
	//	const FNavigationSegmentLink& SegmentLink = SegmentLinks[i];
	//	LinksBB += SegmentLink.LeftStart;
	//	LinksBB += SegmentLink.LeftEnd;
	//	LinksBB += SegmentLink.RightStart;
	//	LinksBB += SegmentLink.RightEnd;
	//}

	LinksBB = LinksBB.TransformBy(RootComponent->GetComponentTransform());

	return LinksBB;
}

#if WITH_EDITORONLY_DATA
void ATacticalVaultMarker::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	static const FName NAME_PointLinks = GET_MEMBER_NAME_CHECKED(ATacticalVaultMarker, PointLinks);

	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;


	bool bUpdateInNavOctree = false;
	//if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ANavLinkProxy, bSmartLinkIsRelevant))
	//{
	//	SmartLinkComp->SetNavigationRelevancy(bSmartLinkIsRelevant);
	//	bUpdateInNavOctree = true;
	//}

	const FName CategoryName = FObjectEditorUtils::GetCategoryFName(PropertyChangedEvent.Property);
	const FName MemberCategoryName = FObjectEditorUtils::GetCategoryFName(PropertyChangedEvent.MemberProperty);
	if (CategoryName == TEXT("SimpleLink") || MemberCategoryName == TEXT("SimpleLink"))
	{
		bUpdateInNavOctree = true;
		if (PropertyName == NAME_PointLinks)
		{
			for (FNavigationLink& Link : PointLinks)
			{
				Link.InitializeAreaClass(/*bForceRefresh=*/true);
			}
		}
	}

	if (bUpdateInNavOctree)
	{
		UNavigationSystem* NavSys = UNavigationSystem::GetCurrent(GetWorld());
		if (NavSys)
		{
			NavSys->UpdateActorInNavOctree(*this);
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR

//void ATacticalVaultMarker::PostInitializeComponents()
//{
//	Super::PostInitializeComponents();
//}
//
//void ATacticalVaultMarker::PostLoad()
//{
//	Super::PostLoad();
//}

#if WITH_EDITORONLY_DATA
/** Returns EdRenderComp subobject **/
UNavLinkRenderingComponent* ATacticalVaultMarker::GetEdRenderComp() const { return EdRenderComp; }
#endif