// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Subsystems/WebPStaticDisplaySubsystem.h"
#include "Subsystems/WebPAnimDisplaySubsystem.h"
#include "Subsystems/WebPScreenshotSubsystem.h"
#include "Subsystems/WebPScreenRecordSubsystem.h"
#include "WebPSupportBPLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnWebPAnimCheckedBP, bool, bSuccess, bool, IsAnimPicture);

UCLASS()
class WEBPSUPPORT_API UWebPSupportBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta=( DisplayName="GetWebPLibraryVersion", Keywords = "Get WebP Library Version"), Category = "WebPSupport")
	static FString GetLibraryVersion();

	UFUNCTION(BlueprintCallable, meta=(WorldContext = "InWorldContextObject", DisplayName="GenerateScreenShotAsync"), Category = "WebPSupport")
	static void GenerateScreenShotAsync(const UObject* InWorldContextObject, const FString& InSavePath, const FOnGenerateWebPScreenshotEndBP& InEndCallback);

	UFUNCTION(BlueprintCallable, meta=(WorldContext = "InWorldContextObject", DisplayName="StartRecordFullScreen"), Category = "WebPSupport")
	static bool StartRecordFullScreen(const UObject* InWorldContextObject, const FString& InSavePath);

	UFUNCTION(BlueprintCallable, meta=(WorldContext = "InWorldContextObject", DisplayName="StartRecordScreenArea"), Category = "WebPSupport")
	static bool StartRecordScreenArea(const UObject* InWorldContextObject, const FString& InSavePath, FIntVector4 RecordArea);

	UFUNCTION(BlueprintCallable, meta=(WorldContext = "InWorldContextObject", DisplayName="EndRecordScreen"), Category = "WebPSupport")
	static void EndRecordScreen(const UObject* InWorldContextObject, const FOnGenerateWebPScreenRecordEndBP& InEndCallback);

	UFUNCTION(BlueprintCallable, meta=(WorldContext = "InWorldContextObject", DisplayName="CheckPictureIsAnim"), Category = "WebPSupport")
	static void CheckPictureIsAnim(const UObject* InWorldContextObject, const FString& InFilePath, const FOnWebPAnimCheckedBP& InResultCallback);

	UFUNCTION(BlueprintCallable, meta=(WorldContext = "InWorldContextObject", DisplayName="LoadWebPStaticPicture"), Category = "WebPSupport")
	static void LoadWebPStaticPicture(const UObject* InWorldContextObject, const FString& InFilePath, const FOnWebPStaticPictureLoadEndBP& InLoadEndCallback);

	UFUNCTION(BlueprintCallable, meta=(WorldContext = "InWorldContextObject", DisplayName="LoadWebPAnimPicture"), Category = "WebPSupport")
	static void LoadWebPAnimPicture(const UObject* InWorldContextObject, const FString& InFilePath, const FOnWebPAnimPictureLoadEndBP& InLoadEndCallback);

	UFUNCTION(BlueprintCallable, meta=(WorldContext = "InWorldContextObject", DisplayName="ReleaseAnimPictureById"), Category = "WebPSupport")
	static void ReleaseAnimPictureById(const UObject* InWorldContextObject, int64 ProcessId);
	
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "InWorldContextObject", DisplayName="ReleaseAnimPictureByTexture"), Category = "WebPSupport")
	static void ReleaseAnimPictureByTexture(const UObject* InWorldContextObject, UTexture2D* WebPTexture);

protected:
	static bool GetWorldContextGameInstance(const UObject* InWorldContextObject, UGameInstance*& OutGameInstance);
};
