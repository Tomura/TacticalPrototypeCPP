// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Engine/NavigationObjectBase.h"
#include "TacticalEnemyStart.generated.h"


UCLASS(Blueprintable, ClassGroup = Common, hidecategories = Collision)
class TACTICALFPS_API ATacticalEnemyStart : public ANavigationObjectBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATacticalEnemyStart(const FObjectInitializer& OI);

	UFUNCTION(Category = "Enemy Start", BlueprintNativeEvent)
		void OnEnemySpawned(class APawn* NewEnemy, class AAIController* NewEnemyController);
	void OnEnemySpawned_Implementation(class APawn* NewEnemy, class AAIController* NewEnemyController) {}

#if WITH_EDITORONLY_DATA
private :
	/** Arrow component to indicate forward direction of start */
	UPROPERTY()
	class UArrowComponent* ArrowComponent;

public:
#endif

#if WITH_EDITORONLY_DATA
	/** Returns ArrowComponent subobject **/
	class UArrowComponent* GetArrowComponent() const { return ArrowComponent; }
#endif


public:
	UPROPERTY(Category = Classes, EditInstanceOnly, BlueprintReadWrite)
	TSubclassOf<ACharacter> EnemyClassOverride;
};
