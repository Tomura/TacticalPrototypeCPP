// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalGameMode.h"
#include "TacticalCharacter.h"
#include "TacticalPlayerController.h"
#include "TacticalAIController.h"
#include "TacticalEnemyStart.h"
#include "TacticalGameState.h"
#include "TacticalPlayerState.h"
#include "TacticalLoadout.h"
#include "TacticalHUD.h"

ATacticalGameMode::ATacticalGameMode(const FObjectInitializer& OI) 
	: Super(OI)
{
	bUseSeamlessTravel = true;
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/ThirdPersonCharacter"));
	//if (PlayerPawnBPClass.Class != NULL)
	//{
	//	DefaultPawnClass = PlayerPawnBPClass.Class;
	//}
	DefaultPawnClass = ATacticalCharacter::StaticClass();
	PlayerControllerClass = ATacticalPlayerController::StaticClass();
	HUDClass = ATacticalHUD::StaticClass();
	DefaultEnemyClass = nullptr;
	GameStateClass = ATacticalGameState::StaticClass();
	PlayerStateClass = ATacticalPlayerState::StaticClass();
	PreRoundWidgetClass = nullptr;

	NumEnemies = 10;

	bPendingNewRound = false;
	bRoundInProgress = false;

	RoundTimeLimit = 0;
	RoundEndTime = 5.f;

	bFriendlyFire = true;
	bSelfDamage = true;

	DamageModifier = 1.f;
}

void ATacticalGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ATacticalGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bPendingNewRound)
	{
		InitRound();
	}
}


void ATacticalGameMode::HandleMatchHasStarted()
{
	ATacticalGameState* GState = Cast<ATacticalGameState>(GameState);
	if (GState)
	{
		GState->RoundTimeLimit = ((float)RoundTimeLimit) * 60.f;
		GState->RoundNumber = 1;
		GState->TimeRoundStart = GetWorld()->GetTimeSeconds();
	}

	GameSession->HandleMatchHasStarted();

	// start human players first
	//for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	//{
	//	APlayerController* PlayerController = *Iterator;
	//	if ((PlayerController->GetPawn() == NULL) && PlayerCanRestart(PlayerController))
	//	{
	//		RestartPlayer(PlayerController);
	//	}
	//}

	InitRound();

	// Make sure level streaming is up to date before triggering NotifyMatchStarted
	GEngine->BlockTillLevelStreamingCompleted(GetWorld());

	// First fire BeginPlay, if we haven't already in waiting to start match
	GetWorldSettings()->NotifyBeginPlay();

	// Then fire off match started
	GetWorldSettings()->NotifyMatchStarted();

	// if passed in bug info, send player to right location
	const FString BugLocString = UGameplayStatics::ParseOption(OptionsString, TEXT("BugLoc"));
	const FString BugRotString = UGameplayStatics::ParseOption(OptionsString, TEXT("BugRot"));
	if (!BugLocString.IsEmpty() || !BugRotString.IsEmpty())
	{
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = Iterator->Get();
			if (PlayerController->CheatManager != NULL)
			{
				PlayerController->CheatManager->BugItGoString(BugLocString, BugRotString);
			}
		}
	}

	if (IsHandlingReplays() && GetGameInstance() != nullptr)
	{
		GetGameInstance()->StartRecordingReplay(GetWorld()->GetMapName(), GetWorld()->GetMapName());
	}
}



void ATacticalGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// If players should start as spectators, leave them in the spectator state
	if (!bStartPlayersAsSpectators && !NewPlayer->PlayerState->bOnlySpectator)
	{
		// If match is in progress, start the player
		if (IsMatchInProgress())
		{
			/*RestartPlayer(NewPlayer);

			if (NewPlayer->GetPawn() != NULL)
			{
				NewPlayer->GetPawn()->ClientSetRotation(NewPlayer->GetPawn()->GetActorRotation());
			}*/

			// todo: Handle stuff

		}
		// Check to see if we should start right away, avoids a one frame lag in single player games
		else if (GetMatchState() == MatchState::WaitingToStart)
		{
			// Check to see if we should start the match
			if (ReadyToStartMatch())
			{
				StartMatch();
			}
		}
	}
}

void ATacticalGameMode::InitializeHUDForPlayer(APlayerController* NewPlayer)
{
	Super::InitializeHUDForPlayer(NewPlayer);
	// tell client what hud class to use
	NewPlayer->ClientSetHUD(HUDClass);
}

void ATacticalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	int32 ParsedEnemies = FCString::Atoi(*UGameplayStatics::ParseOption(Options, TEXT("NumEnemies")));
	UE_LOG(LogTemp, Log, TEXT("Enemies to Spawn: %d"), ParsedEnemies);
	if (ParsedEnemies > 0)
	{
		NumEnemies = ParsedEnemies;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid EnemyNumber in Options. Using Default (&d)"), NumEnemies);
	}
}

APlayerController* ATacticalGameMode::Login(class UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	APlayerController* Result = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
	if (Result)
	{
		// todo: Character skins, etc
	}

	return Result;
}

void ATacticalGameMode::RestartRound()
{
	InitRound(true);
}

void ATacticalGameMode::InitRound(bool bRestart /*= false*/)
{
	if (IsPendingKill())
		return;

	// Reset everything
	for (TActorIterator<AAIController> It(GetWorld()); It; ++It)
	{
		const FString ControllerName = (*It)->GetFullName();
		AAIController* AIC = Cast<AAIController>(*It);
		if (AIC)
		{
			AIC->Destroy();
		}
	}

	// Call Reset on all other Pawns
	ResetLevel();

	SpawnEnemies();

	ATacticalGameState* GState = Cast<ATacticalGameState>(GameState);
	if (GState)
	{
		GState->bWaitingForNextRound = true;
		// Increment Round Number unless we are restarting the round
		if (!bRestart)
		{
			++GState->RoundNumber;
		}
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATacticalPlayerController* TPC = Cast<ATacticalPlayerController>(*It);
		if (TPC)
		{
			// Set All Players to "not ready"
			TPC->ServerSetReady(false);
			TPC->ClientResetLevel();
			//TPC->PlayerS
		}
	}

	bPendingNewRound = false;
	
	// Restart Players
	// BeginRound();
	// todo: don't call begin round immediately. Display Preparation UI and way for Ready/time-up
}

void ATacticalGameMode::BeginRound()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		RestartPlayer(It->Get());
	}

	if (ATacticalGameState* GState = Cast<ATacticalGameState>(GameState))
	{
		GState->bWaitingForNextRound = false;
		GState->TimeRoundStart = GetWorld()->GetTimeSeconds();
		GState->OnBeginRound();
	}
	if(RoundTimeLimit > 0)
		GetWorldTimerManager().SetTimer(TimerEndRound, this, &ATacticalGameMode::EndRound, (float)(RoundTimeLimit*60), false);
	bRoundInProgress = true;
}

void ATacticalGameMode::EndRound()
{
	bPendingNewRound = true;
	bRoundInProgress = false;

	// clear previous timers
	GetWorldTimerManager().ClearTimer(TimerNewRound);
	GetWorldTimerManager().ClearTimer(TimerEndRound);

	if (ATacticalGameState* GState = Cast<ATacticalGameState>(GameState))
	{
		GState->bWaitingForEndRound = false;
		GState->TimeRoundEnd = GetWorld()->GetTimeSeconds();
	}
}

void ATacticalGameMode::PlayerBecameReady(class ATacticalPlayerController* ReadyPC)
{
	if (bRoundInProgress)
	{
		//ReadyPC->ServerSetReady(true);
		return;
	}

	bool bEveryOneReady = true;

	// todo: check if we are even waiting

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATacticalPlayerController* TPC = Cast<ATacticalPlayerController>(*It);
		if (TPC)
		{
			ATacticalPlayerState* TPState = Cast<ATacticalPlayerState>(TPC->PlayerState);
			if (TPState)
			{
				if (TPState->bReady == false && !TPState->IsOnlySpecator())
				{
					bEveryOneReady = false;
					break;
				}
			}
		}
	}

	if (bEveryOneReady)
	{
		BeginRound();
	}
}

void ATacticalGameMode::Killed(class AController* Killer, class AController* Victim, class AActor* KilledActor, const class UDamageType* DamageType)
{
	// todo: Kill Messaging, etc
	Super::Killed(Killer, Victim, KilledActor, DamageType);
	
	EndRoundCheck();
}

bool ATacticalGameMode::AllPlayersDead() const
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = Cast<APlayerController>(*It))
		{
			ATacticalCharacter* TacPawn = Cast<ATacticalCharacter>(PC->GetPawn());
			if (TacPawn)
			{
				return false;
			}
		}
	}
	return true;
}

bool ATacticalGameMode::AllEnemiesDead() const
{
	for (TActorIterator<AAIController> It(GetWorld()); It; ++It)
	{
		const FString ControllerName = (*It)->GetFullName();
		AAIController* AIC = Cast<AAIController>(*It);
		if (AIC)
		{
			if (AIC->GetPawn())
			{
				ATacticalCharacter* AIPawn = Cast<ATacticalCharacter>(AIC->GetPawn());
				if (AIPawn && AIPawn->IsAlive())
				{
					return false;
				}
			}
		}
	}
	return true;
}



void ATacticalGameMode::EndRoundCheck()
{
	if (!bRoundInProgress || bPendingNewRound)
		return;

	bool bEndRoundWin = AllEnemiesDead();
	bool bEndRoundLoss = AllPlayersDead();


	if (bEndRoundLoss)
	{
		EndRound();
	}
	else if (bEndRoundWin)
	{
		// clear previous timers
		GetWorldTimerManager().ClearTimer(TimerNewRound);
		GetWorldTimerManager().ClearTimer(TimerEndRound);
		// set timer before the round ends
		GetWorldTimerManager().SetTimer(TimerNewRound, this, &ATacticalGameMode::EndRound, RoundEndTime, false);

		// todo: Display UI Message ("Round Successful. Starting new round in ... 3, 2, 1")
		if (ATacticalGameState* GState = Cast<ATacticalGameState>(GameState))
		{
			GState->TimeRoundEnd = GetWorld()->GetTimeSeconds();
			GState->bWaitingForEndRound = true;
		}
	}
}

void ATacticalGameMode::SpawnEnemies()
{
	if (DefaultEnemyClass != nullptr && DefaultEnemyClass->IsValidLowLevel() && NumEnemies > 0)
	{
		TArray<ATacticalEnemyStart*> EnemyStarts;
		// Get Enemy Starts
		for (TActorIterator<ATacticalEnemyStart> It(GetWorld()); It; ++It)
		{
			EnemyStarts.Add(*It);
		}

		TArray<ATacticalEnemyStart*> AvailableStarts;
		AvailableStarts.Append(EnemyStarts);
		if (EnemyStarts.Num() <= 0)
		{
			return;
		}

		// Limit Enemies to Number of Starts or else it will crash
		NumEnemies = FMath::Clamp(NumEnemies, 1, EnemyStarts.Num());

		for (int32 i = 0; i < NumEnemies; i++)
		{
			if (AvailableStarts.Num() > 0)
			{
				int32 StartIdx = FMath::RandRange(0, AvailableStarts.Num() - 1);
				if (SpawnEnemy(AvailableStarts[StartIdx]))
				{
					AvailableStarts.RemoveAt(StartIdx);
				}
			}
			else
			{
				// Commented out because it will crash
				//int32 StartIdx = FMath::RandRange(0, EnemyStarts.Num());
				//SpawnEnemy(EnemyStarts[StartIdx]);
			}
		}
	}
}

APawn* ATacticalGameMode::SpawnEnemy(ATacticalEnemyStart* StartSpot)
{	
	if (!StartSpot)
		return nullptr;

	TSubclassOf<ACharacter> EnemyClass = nullptr;
	const int32 NumEnemyClasses = SpawnEnemyClass.Num();
	if (NumEnemyClasses < 1)
	{
		EnemyClass = DefaultEnemyClass;
	}
	else
	{
		const int32 SpawnIdx = FMath::RandRange(0, NumEnemyClasses - 1);
		EnemyClass = SpawnEnemyClass[SpawnIdx];
	}

	//TSubclassOf<ACharacter> EnemyClass = StartSpot->EnemyClassOverride ? StartSpot->EnemyClassOverride : DefaultEnemyClass;
	if (!EnemyClass)
	{		
		return nullptr;
	}
	
	// don't allow pawn to be spawned with any pitch or roll
	FRotator StartRotation(ForceInit);
	StartRotation.Yaw = StartSpot->GetActorRotation().Yaw;
	FVector StartLocation = StartSpot->GetActorLocation();


	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = Instigator;
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save default player pawns into a map
	APawn* ResultPawn = GetWorld()->SpawnActor<APawn>(EnemyClass, StartLocation, StartRotation, SpawnInfo);
	if (ResultPawn == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Couldn't spawn Enemy of type %s at %s"), *GetNameSafe(EnemyClass), *StartSpot->GetName());
		return nullptr;
	}

	AAIController* NewAI = GetWorld()->SpawnActor<AAIController>(ResultPawn->AIControllerClass, StartLocation, StartRotation, SpawnInfo);
	if (NewAI)
	{
		NewAI->Possess(ResultPawn);
	}

	StartSpot->OnEnemySpawned(ResultPawn, NewAI);
	// todo: Setup AI?

	return ResultPawn;
}
