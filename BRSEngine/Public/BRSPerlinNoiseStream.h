// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "Object.h"
#include "BRSPerlinNoiseStream.generated.h"


USTRUCT()
struct FBRSPerlinNoiseOctave
{
	GENERATED_BODY()
public:
	FBRSPerlinNoiseOctave()
		: Order(1),
		val1(0.f),
		val2(0.f),
		time1(0.f),
		time2(1.f),
		weight(0.f),
		Seed(0),
		RandomStream(FRandomStream(ForceInit))
	{}

	FBRSPerlinNoiseOctave(uint8 inOrder, float TimeInterval, float inWeight, int32 RandomSeed, float InitialTime = 0.f)
	{
		Init(inOrder, TimeInterval, inWeight, RandomSeed, InitialTime);
	}

	void Init(float TimeInterval, float InitialTime = 0.f)
	{
		RandomStream = FRandomStream(Seed);
		if (Order <= 0)
		{
			Order = 1;
		}
		time1 = InitialTime;
		time2 = InitialTime + (TimeInterval / (float(Order)));
		val1 = FMath::FRand();
		val2 = FMath::FRand();
	}

	void Init(uint8 inOrder, float TimeInterval, float inWeight, int32 RandomSeed, float InitialTime = 0.f)
	{
		Order = inOrder;
		weight = inWeight;
		Seed = RandomSeed;
		Init(TimeInterval, InitialTime);
	}

	FORCEINLINE void SetWeight(float inWeight)
	{
		weight = inWeight;
	}

	FORCEINLINE float WeightingFct(float t)
	{
		// could be optimized by t*t*t... instead of pow, because there is no Generic FMath::Pow(float, int)
		return (6.f * FMath::Pow(t, 5.f) - 15.f*FMath::Pow(t, 4.f) + 10 * FMath::Pow(t, 3.f));
	}

	FORCEINLINE float GetSample(float t)
	{
		if (t >= time2)
		{
			//UE_LOG(LogTemp, Log, TEXT("New Value"));
			while (t >= time2)
			{
				const float time2add = OctPntInterval();
				// Shift end values to start
				time1 = time2;
				val1 = val2;

				// generate new end values
				time2 += time2add;
				val2 = FMath::FRand();
			}
		}

		return (WeightingFct((t-time1)/OctPntInterval())*(val2 - val1) + val1);
	}

	FORCEINLINE float GetWeight() { return weight; }

	FORCEINLINE float OctPntInterval() { return (time2 - time1); }


protected:
	FRandomStream RandomStream;
	UPROPERTY(EditAnywhere, Category = Data)
	int32 Seed;

	UPROPERTY(EditAnywhere, Category = Data)
	uint8 Order;
	float val1;
	float val2;
	float time1;
	float time2;
	UPROPERTY(EditAnywhere, Category = Data)
	float weight;
};

USTRUCT(BlueprintType)
struct FBRSPerlinNoiseStream1D
{
	GENERATED_BODY()
public:
	FBRSPerlinNoiseStream1D()
		: Interval(1.f)
	{
		bInitialized = false;
		Octaves.Empty();
	}
	//FBRSPerlinNoiseStream1D(uint8 NumOctaves, float inTimeInterval)
	//	: Interval(1.f)
	//{
	//	bInitialized = false;
	//	Interval = inTimeInterval;
	//	Octaves.Empty();
	//	for (int32 i = 1; i < (NumOctaves + 1); i++)
	//	{
	//		Octaves.Add(FBRSPerlinNoiseOctave(i, Interval, FMath::Pow(0.5f, i)));
	//	}
	//}

	void InitOctaves()
	{
		for (FBRSPerlinNoiseOctave& oct : Octaves)
		{
			oct.Init(Interval);
		}
		bInitialized = true;
	}

	//void AddOctave(uint8 Order, float inWeight)
	//{
	//	Octaves.Add(FBRSPerlinNoiseOctave(Order, Interval, inWeight));
	//}
		
	float Sample(float Time)
	{
		if (!bInitialized)
		{
			InitOctaves();
		}
		if (Octaves.Num() > 0)
		{
			float Result = 0.f;
			float SumWeight = 0.f;
			for (FBRSPerlinNoiseOctave& oct : Octaves)
			{
				Result += oct.GetWeight() * oct.GetSample(Time);
				SumWeight += oct.GetWeight();
			}
			if (SumWeight > 0.f)
			{
				Result /= SumWeight;
				return Result;
			}
			else
			{
				return 0.f;
			}
		}
		return 0.f;
	}

protected:
	bool bInitialized;
	UPROPERTY(EditAnywhere, Category = Data)
	TArray<FBRSPerlinNoiseOctave> Octaves;
	UPROPERTY(EditAnywhere, Category = Data)
	float Interval;
};
