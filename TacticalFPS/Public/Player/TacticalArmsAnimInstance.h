// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Animation/AnimInstance.h"
#include "TacticalArmsAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalArmsAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(Category = Animation, BlueprintReadWrite, EditAnywhere)
		UAnimSequence* IdleAnimation;

	UPROPERTY(Category = Animation, BlueprintReadWrite, EditAnywhere)
		UAnimSequence* IdleAnimation_Left;
	
	
};
