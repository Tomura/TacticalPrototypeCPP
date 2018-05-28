// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "BRSChatWindowUMG_Base.generated.h"


UENUM(Blueprintable)
namespace EStoredChatMessageType
{
	enum Type
	{
		All,
		Team,
		Server
	};
}

/**
 * 
 */
UCLASS(Config = Game, abstract)
class BRSENGINE_API UBRSChatWindowUMG_Base : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UBRSChatWindowUMG_Base(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Chat)
	void ReceiveNewMessage(const FString& MessageSender, const FString& Message, EStoredChatMessageType::Type MessageType);

	UPROPERTY(BlueprintReadOnly, GlobalConfig, Category = Chat)
	int32 MaxStoredMessages;

	UPROPERTY(BlueprintReadOnly, GlobalConfig, Category = Chat)
	float MessageLifeTimeSeconds;
};
