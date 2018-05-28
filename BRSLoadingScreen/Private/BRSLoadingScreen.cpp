// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSLoadingScreen.h"
#include "MoviePlayer.h"
#include "SBRSLoadingScreen.h"

// This module must be loaded "PreLoadingScreen" in the .uproject file, otherwise it will not hook in time!


class FBRSLoadingScreenModule : public IBRSLoadingScreenModule
{
public:
	FDelegateHandle StartupMoviesFinishedHandle;

	virtual void StartupModule() override
	{		
		// Load for cooker reference
		LoadObject<UObject>(NULL, TEXT("/Game/UI/LoadingScreen/LoadingScreen.LoadingScreen") );

		if (IsMoviePlayerEnabled())
		{
			//FLoadingScreenAttributes LoadingScreen;
			//LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
			//LoadingScreen.MoviePaths.Add(TEXT("LogoIntro2"));
			GetMoviePlayer()->SetupLoadingScreenFromIni();
			StartupMoviesFinishedHandle = GetMoviePlayer()->OnMoviePlaybackFinished().AddRaw(this, &FBRSLoadingScreenModule::OnStartupMoviesFinished);
		}
	}

	void OnStartupMoviesFinished()
	{
		UE_LOG(LogTemp, Log, TEXT("Movies Finished"));
		GetMoviePlayer()->OnMoviePlaybackFinished().Remove(StartupMoviesFinishedHandle);
		// Movies are finished so start loading screen widget
		StartInGameLoadingScreen();
	}
	
	virtual bool IsGameModule() const override
	{
		return true;
	}

	virtual void StartInGameLoadingScreen() override
	{
		if (IsMoviePlayerEnabled() && GetMoviePlayer() && !GetMoviePlayer()->IsMovieCurrentlyPlaying())
		{
			FLoadingScreenAttributes LoadingScreen;
			LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
			LoadingScreen.WidgetLoadingScreen = SNew(SBRSLoadingScreen2);
			GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
		}
	}

};

IMPLEMENT_GAME_MODULE(FBRSLoadingScreenModule, BRSLoadingScreen);
