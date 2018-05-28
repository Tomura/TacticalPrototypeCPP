// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "TacticalLevelSummary.generated.h"


UCLASS()
class TACTICALFPS_API UTacticalLevelSummary : public UObject
{
	GENERATED_BODY()
public:
	UTacticalLevelSummary(const FObjectInitializer& ObjectInitializer);

	/** name of author or authors */
	UPROPERTY(EditInstanceOnly, Category = LevelSummary)
	FString Author;

	/** map Discription */
	UPROPERTY(EditInstanceOnly, Category = LevelSummary)
	FText MapTitle;

	/** map Discription */
	UPROPERTY(EditInstanceOnly, Category = LevelSummary)
	FText Description;

	/** Custom loading screen image */
	UPROPERTY(EditInstanceOnly, Category = LevelSummary)
	UTexture2D* LoadingScreenImage;

	/** image that is displayed on the map selection screen's preview window */
	UPROPERTY(EditInstanceOnly, Category = LevelSummary)
	UTexture2D* PreviewImage;

	UPROPERTY(EditInstanceOnly, Category = LevelSummary)
	FIntPoint RecommendedPlayerCount;
};