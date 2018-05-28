// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalCoverMarker.h"
#include "NavArea_Cover.h"



const float ATacticalCoverMarker::CoverHalfHeight_High(80.f);
const float ATacticalCoverMarker::CoverHalfHeight_Low(40.f);
const float ATacticalCoverMarker::CoverHalfDepth(25.f);



// Sets default values
ATacticalCoverMarker::ATacticalCoverMarker()
{
#if WITH_EDITORONLY_DATA
	// Statics (names, path, etc for constructor)
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> CoverSpriteTextureObject;
		FConstructorStatics()
			: CoverSpriteTextureObject(TEXT("/Game/Icons/T_CoverMarker"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;
#endif

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Length = 500.f;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionObjectType(ECC_Helper);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetCanEverAffectNavigation(true);
	CollisionBox->bDynamicObstacle = true;
	CollisionBox->AreaClass = UNavArea_CoverHigh::StaticClass();
	CollisionBox->SetBoxExtent(FVector(CoverHalfDepth, 0.5f * Length, CoverHalfHeight_High), false);
	CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, CoverHalfHeight_High));
	CollisionBox->ShapeColor = FColor::Green;

	CoverType = ECoverType::HighCover;

#if WITH_EDITORONLY_DATA
	if (ConstructorStatics.CoverSpriteTextureObject.Get())
	{
		Sprite = nullptr;
		Sprite = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
		if (Sprite)
		{
			Sprite->SetupAttachment(RootComponent);
			Sprite->SetSprite(ConstructorStatics.CoverSpriteTextureObject.Get());
			Sprite->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
		}
	}

	Arrow = CreateAbstractDefaultSubobject<UArrowComponent>(TEXT("ForwardArrow"));
	if (Arrow)
	{
		Arrow->SetupAttachment(RootComponent);
		Arrow->SetRelativeLocation(FVector(-50.f, 0.f, 0.f));
		Arrow->SetArrowColor(FLinearColor::Green);
		Arrow->ArrowSize = 1.f;
	}
#endif

	bDebugReservations = false;
}

// Called when the game starts or when spawned
void ATacticalCoverMarker::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATacticalCoverMarker::Reset()
{
	Super::Reset();
	Reservations.Empty();
}

// Called every frame
void ATacticalCoverMarker::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void ATacticalCoverMarker::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Length = FMath::Max(50.f, Length);

	if (CollisionBox)
	{
		const bool bLowCover = (CoverType == ECoverType::LowCover);
		FVector DesiredExtent;
		DesiredExtent.X = CoverHalfDepth;
		DesiredExtent.Y = 0.5f*Length;
		DesiredExtent.Z = bLowCover ? CoverHalfHeight_Low : CoverHalfHeight_High;
		CollisionBox->SetBoxExtent(DesiredExtent);
		CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, DesiredExtent.Z));
		CollisionBox->AreaClass = bLowCover ? UNavArea_CoverLow::StaticClass() : UNavArea_CoverHigh::StaticClass();
	}
}

FRotator ATacticalCoverMarker::GetDesiredRotation() const
{
	return GetActorRotation();
}


// Reserves a spot in cover if possible. Returns true on success and false on failure.
// This function also handles the update of all registered reservations and will remove expired reservations
bool ATacticalCoverMarker::ReserveCover(AActor* ReservingActor, const FVector& Location)
{
	const float MaxDist = 200.f;
	const float MaxDistSq = MaxDist*MaxDist; // squared distance between reservations in uu^2
	const float ReservationDuration = 15.f; // duration of reservation in seconds

	const float Time = GetWorld()->GetTimeSeconds();

	if (IsCoverLocationFree(ReservingActor, Location))
	{
		FVector SavedLoc = Location;
		SavedLoc.Z = Time;
		Reservations.Add(ReservingActor, Location);
		if (bDebugReservations)
		{
			DrawDebugSphere(GetWorld(), Location, MaxDist * 0.5f, 32, FColor::Red, false, ReservationDuration, (uint8)'\000', 2.5f);
		}

		return true;
	}

	return false;
}



bool ATacticalCoverMarker::IsCoverLocationFree(AActor* ReservingActor, const FVector& Location)
{
	const float MaxDist = 200.f;
	const float MaxDistSq = MaxDist*MaxDist; // squared distance between reservations in uu^2
	const float ReservationDuration = 15.f; // duration of reservation in seconds

	if (ReservingActor == nullptr)
	{
		return false;
	}


	const float Time = GetWorld()->GetTimeSeconds();
	bool bCanReserve = true;

	TArray<AActor*> KeysToRemove;

	// test reservations
	for (const auto& Entry : Reservations)
	{

		const float ReservedTime = Entry.Value.Z;

		if ((Time - ReservedTime) <= ReservationDuration)
		{
			AActor* Reservee = Entry.Key;
			FVector ReservedLoc = Entry.Value;
			ReservedLoc.Z = 0.f;

			FVector Delta = (ReservedLoc - Location);
			Delta.Z = 0.f;

			// if there is one cover that is blocking reservation don't
			if ((Reservee != ReservingActor) && (Delta.SizeSquared2D() < MaxDistSq))
			{
				bCanReserve = false;
			}
		}
		else
		{
			KeysToRemove.AddUnique(Entry.Key);
		}
	}

	// remove keys that are pending for removal
	for (auto Key : KeysToRemove)
	{
		Reservations.Remove(Key);
	}

	if (bCanReserve)
	{
		// test occupying actors
		TArray<AActor*> Overlapping;
		GetOverlappingActors(Overlapping, ACharacter::StaticClass());
		for (AActor* TestActor : Overlapping)
		{
			if (ACharacter* TestChar = Cast<ACharacter>(TestActor))
			{
				const FVector TestLocation = TestChar->GetActorLocation();
				FVector Delta = (TestLocation - Location);
				Delta.Z = 0.f;
				if ((TestChar->GetController() != ReservingActor)
					&& (TestChar != ReservingActor)
					&& (Delta.SizeSquared2D() < MaxDistSq))
				{
					bCanReserve = false;
				}
			}
		}
	}

	return bCanReserve;
}

bool ATacticalCoverMarker::IsCoverLocationFreeConst(AActor* ReservingActor, const FVector& Location) const
{
	const float MaxDist = 200.f;
	const float MaxDistSq = MaxDist*MaxDist; // squared distance between reservations in uu^2
	const float ReservationDuration = 15.f; // duration of reservation in seconds

	if (ReservingActor == nullptr)
	{
		return false;
	}

	const float Time = GetWorld()->GetTimeSeconds();
	bool bCanReserve = true;

	// test reservations
	for (const auto& Entry : Reservations)
	{

		const float ReservedTime = Entry.Value.Z;

		if ((Time - ReservedTime) <= ReservationDuration)
		{
			AActor* Reservee = Entry.Key;
			FVector ReservedLoc = Entry.Value;
			ReservedLoc.Z = 0.f;

			FVector Delta = (ReservedLoc - Location);
			Delta.Z = 0.f;

			// if there is one cover that is blocking reservation don't
			if ((Reservee != ReservingActor) && (Delta.SizeSquared2D() < MaxDistSq))
			{
				bCanReserve = false;
			}
		}
	}

	if (bCanReserve)
	{
		// test occupying actors
		TArray<AActor*> Overlapping;
		GetOverlappingActors(Overlapping, ACharacter::StaticClass());
		for (AActor* TestActor : Overlapping)
		{
			if (ACharacter* TestChar = Cast<ACharacter>(TestActor))
			{
				const FVector TestLocation = TestChar->GetActorLocation();
				FVector Delta = (TestLocation - Location);
				Delta.Z = 0.f;
				if ((TestChar->GetController() != ReservingActor)
					&& (TestChar != ReservingActor)
					&& (Delta.SizeSquared2D() < MaxDistSq))
				{
					bCanReserve = false;
				}
			}
		}
	}

	return bCanReserve;
}

void ATacticalCoverMarker::CancelReservation(AActor* InActor)
{
	if (Reservations.Contains(InActor))
	{
		Reservations.Remove(InActor);
	}
}

ECoverAimType ATacticalCoverMarker::GetAimType(const FVector& Location) const
{
	const FVector RelativeLoc = GetActorTransform().InverseTransformPosition(Location);
	if (GetCoverType() == ECoverType::LowCover)
	{
		return ECoverAimType::AimUp;
	}
	return ECoverAimType::None;
}

float ATacticalCoverMarker::GetDistance2D(const FVector& Location) const
{
	const FVector ClosestPoint = FMath::ClosestPointOnLine(Location, 
		GetActorLocation() - 0.5f * GetActorRightVector() * Length, 
		GetActorLocation() + 0.5f * GetActorRightVector() * Length);
	return (Location - ClosestPoint).Size2D();
}

float ATacticalCoverMarker::GetDistanceSquared2D(const FVector& Location) const
{
	const FVector ClosestPoint = FMath::ClosestPointOnLine(Location,
		GetActorLocation() - 0.5f * GetActorRightVector() * Length,
		GetActorLocation() + 0.5f * GetActorRightVector() * Length);
	return (Location - ClosestPoint).SizeSquared2D();
}
