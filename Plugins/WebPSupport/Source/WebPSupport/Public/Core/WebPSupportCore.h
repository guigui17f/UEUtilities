// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class WEBPSUPPORT_API WebPSupportCore
{
public:
	// notice: this function will block the thread
	static bool GenerateStaticPicture(const FString& InSavePath, const TArray<FColor>& InColorData, int32 Width, int32 Height, float QualityFactor = 100.f);

	// notice: this function will block the thread
	static bool GenerateAnimPicture(const FString& InSavePath, const TArray<TArray<FColor>>& InColorDataArray, const TArray<int32>& InTimestampsMs, int32 Width, int32 Height, float QualityFactor = 100.f);

	static bool CheckSavePathLegality(const FString& InSavePath);

	static bool GetViewportSize(const UWorld* InWorld, FVector2D& OutViewportSize);
	
	static bool LoadStaticPicture(const FString& InFilePath, TArray<FColor>& OutColorData, int32& OutWidth, int32& OutHeight);
	
	static bool LoadAnimPicture(const FString& InFilePath, TArray<TArray<FColor>>& OutColorDataArray, TArray<int32>& OutTimestampsMs, int32& OutWidth, int32& OutHeight);

	static bool CheckIsAnimPicture(const FString& InFilePath, bool& OutIsAnim);
	
	static bool CheckLoadPathLegality(const FString& InFilePath);
};
