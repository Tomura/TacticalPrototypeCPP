// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/Actor.h"
#include "TacticalTypes.h"
#include "TacticalCoverMarker.generated.h"



UCLASS()
class TACTICALFPS_API ATacticalCoverMarker : public AActor
{
	GENERATED_BODY()


private:
	// Components
	UPROPERTY()
		class UBoxComponent* CollisionBox;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	class UBillboardComponent* Sprite;

	UPROPERTY()
	class UArrowComponent* Arrow;
#endif


public:	
	// Sets default values for this actor's properties
	ATacticalCoverMarker();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Reset() override;

	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(Category = "Cover", BlueprintCallable)
	FRotator GetDesiredRotation() const;


	UFUNCTION(Category = "Cover | AI", BlueprintCallable)
	bool ReserveCover(AActor* ReservingActor, const FVector& Location);

	UFUNCTION(Category = "Cover | AI", BlueprintCallable)
		bool IsCoverLocationFree(AActor* ReservingActor, const FVector& Location);
	UFUNCTION(Category = "Cover | AI", BlueprintCallable)
		bool IsCoverLocationFreeConst(AActor* ReservingActor, const FVector& Location) const;

	UFUNCTION(Category = "Cover | AI", BlueprintCallable)
		void CancelReservation(AActor* InActor);


	ECoverType GetCoverType() const { return CoverType; }
	
	ECoverAimType GetAimType(const FVector& Location) const;

	float GetDistance2D(const FVector& Location) const;
	float GetDistanceSquared2D(const FVector& Location) const;
	
	bool bDebugReservations;

public:
	// Constants
	static const float CoverHalfHeight_High;
	static const float CoverHalfHeight_Low;
	static const float CoverHalfDepth;

protected:	
	UPROPERTY(Category = "Cover", EditInstanceOnly, meta = (DisplayName = "Type"))
	ECoverType CoverType;

	UPROPERTY(Category = "Cover", EditInstanceOnly, meta = (DisplayName = "Length", MinValue=50.0))
	float Length;

	TMap<AActor*, FVector> Reservations;
	
};
