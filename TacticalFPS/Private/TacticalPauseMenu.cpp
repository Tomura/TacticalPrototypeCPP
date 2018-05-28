// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalPauseMenu.h"
#include "TacticalLevelScriptActor.h"


ALevelScriptActor* UTacticalPauseMenu::GetLevelScriptActor() const
{
	if (GetOwningPlayer() && GetOwningPlayer()->GetWorld())
	{
		return GetOwningPlayer()->GetWorld()->GetLevelScriptActor();
	}
	return nullptr;
}

void UTacticalPauseMenu::LevelScriptUIEvent(FName EventName)
{
	ALevelScriptActor* LevelScript = GetLevelScriptActor();
	if (LevelScript)
	{
		ATacticalLevelScriptActor* TacLevelScript = Cast<ATacticalLevelScriptActor>(LevelScript);
		if (TacLevelScript)
		{
			TacLevelScript->OnGenericUIEvent(GetOwningPlayer(), EventName);
		}
	}
}
