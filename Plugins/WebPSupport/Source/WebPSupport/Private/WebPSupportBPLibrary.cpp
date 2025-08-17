// Copyright 2025 guigui17f. All Rights Reserved.

#include "WebPSupportBPLibrary.h"
#include "encode.h"
#include "decode.h"
#include "mux.h"
#include "demux.h"
#include "Core/WebPSupportCore.h"
#include "Subsystems/WebPStaticDisplaySubsystem.h"
#include "Subsystems/WebPAnimDisplaySubsystem.h"
#include "Subsystems/WebPScreenshotSubsystem.h"
#include "Subsystems/WebPScreenRecordSubsystem.h"
#include "Log/WebPSupportLog.h"

FString UWebPSupportBPLibrary::GetLibraryVersion()
{
	int32 EncoderVersion = WebPGetEncoderVersion();
	int32 DecoderVersion = WebPGetDecoderVersion();
	int32 MuxVersion = WebPGetMuxVersion();
	int32 DemuxVersion = WebPGetDemuxVersion();
	FString VersionInfo = FString::Printf(TEXT("EncoderVersion:%d, DecoderVersion:%d, MuxVersion:%d, DemuxVersion:%d"), EncoderVersion, DecoderVersion, MuxVersion, DemuxVersion);
	return VersionInfo;
}

void UWebPSupportBPLibrary::GenerateScreenShotAsync(const UObject* InWorldContextObject, const FString& InSavePath, const FOnGenerateWebPScreenshotEndBP& InEndCallback)
{
	UGameInstance* GameInstance = nullptr;
	if (!GetWorldContextGameInstance(InWorldContextObject, GameInstance))
	{
		InEndCallback.ExecuteIfBound(false);
		return;
	}

	UWebPScreenshotSubsystem* ScreenshotSubsystem = GameInstance->GetSubsystem<UWebPScreenshotSubsystem>();
	return ScreenshotSubsystem->GenerateScreenShotAsync(InSavePath, InEndCallback);
}

bool UWebPSupportBPLibrary::StartRecordFullScreen(const UObject* InWorldContextObject, const FString& InSavePath)
{
	UGameInstance* GameInstance = nullptr;
	if (!GetWorldContextGameInstance(InWorldContextObject, GameInstance))
	{
		return false;
	}

	UWebPScreenRecordSubsystem* ScreenRecordSubsystem = GameInstance->GetSubsystem<UWebPScreenRecordSubsystem>();
	bool bSuccess = ScreenRecordSubsystem->StartRecordScreen(InSavePath);
	return bSuccess;
}

bool UWebPSupportBPLibrary::StartRecordScreenArea(const UObject* InWorldContextObject, const FString& InSavePath, FIntVector4 RecordArea)
{
	UGameInstance* GameInstance = nullptr;
	if (!GetWorldContextGameInstance(InWorldContextObject, GameInstance))
	{
		return false;
	}

	UWebPScreenRecordSubsystem* ScreenRecordSubsystem = GameInstance->GetSubsystem<UWebPScreenRecordSubsystem>();
	bool bSuccess = ScreenRecordSubsystem->StartRecordScreen(InSavePath, RecordArea);
	return bSuccess;
}

void UWebPSupportBPLibrary::EndRecordScreen(const UObject* InWorldContextObject, const FOnGenerateWebPScreenRecordEndBP& InEndCallback)
{
	UGameInstance* GameInstance = nullptr;
	if (!GetWorldContextGameInstance(InWorldContextObject, GameInstance))
	{
		InEndCallback.ExecuteIfBound(false);
		return;
	}

	UWebPScreenRecordSubsystem* ScreenRecordSubsystem = GameInstance->GetSubsystem<UWebPScreenRecordSubsystem>();
	ScreenRecordSubsystem->EndRecordScreen(InEndCallback);
}

void UWebPSupportBPLibrary::CheckPictureIsAnim(const UObject* InWorldContextObject, const FString& InFilePath, const FOnWebPAnimCheckedBP& InResultCallback)
{
	UGameInstance* GameInstance = nullptr;
	if (!GetWorldContextGameInstance(InWorldContextObject, GameInstance))
	{
		InResultCallback.ExecuteIfBound(false, false);
		return;
	}

	AsyncTask(ENamedThreads::AnyThread, [GameInstance, InFilePath, InResultCallback]()
	{
		if (!IsValid(GameInstance))
		{
			return;
		}

		bool bIsAnimPicture;
		bool bSuccess = WebPSupportCore::CheckIsAnimPicture(InFilePath, bIsAnimPicture);

		if (!IsValid(GameInstance))
		{
			return;
		}
		AsyncTask(ENamedThreads::GameThread, [bSuccess, bIsAnimPicture, InResultCallback]()
		{
			if (bSuccess)
			{
				InResultCallback.ExecuteIfBound(true, bIsAnimPicture);
			}
			else
			{
				InResultCallback.ExecuteIfBound(false, false);
			}
		});
	});
}

void UWebPSupportBPLibrary::LoadWebPStaticPicture(const UObject* InWorldContextObject, const FString& InFilePath, const FOnWebPStaticPictureLoadEndBP& InLoadEndCallback)
{
	UGameInstance* GameInstance = nullptr;
	if (!GetWorldContextGameInstance(InWorldContextObject, GameInstance))
	{
		return;
	}
	UWebPStaticDisplaySubsystem* StaticDisplaySubsystem = GameInstance->GetSubsystem<UWebPStaticDisplaySubsystem>();
	StaticDisplaySubsystem->LoadWebPStaticPicture(InFilePath, InLoadEndCallback);
}

void UWebPSupportBPLibrary::LoadWebPAnimPicture(const UObject* InWorldContextObject, const FString& InFilePath, const FOnWebPAnimPictureLoadEndBP& InLoadEndCallback)
{
	UGameInstance* GameInstance = nullptr;
	if (!GetWorldContextGameInstance(InWorldContextObject, GameInstance))
	{
		return;
	}
	UWebPAnimDisplaySubsystem* AnimDisplaySubsystem = GameInstance->GetSubsystem<UWebPAnimDisplaySubsystem>();
	AnimDisplaySubsystem->LoadWebPAnimPicture(InFilePath, InLoadEndCallback);
}

void UWebPSupportBPLibrary::ReleaseAnimPictureById(const UObject* InWorldContextObject, int64 ProcessId)
{
	UGameInstance* GameInstance = nullptr;
	if (!GetWorldContextGameInstance(InWorldContextObject, GameInstance))
	{
		return;
	}
	UWebPAnimDisplaySubsystem* AnimDisplaySubsystem = GameInstance->GetSubsystem<UWebPAnimDisplaySubsystem>();
	AnimDisplaySubsystem->ReleaseWebPAnimPicture(ProcessId);
}

void UWebPSupportBPLibrary::ReleaseAnimPictureByTexture(const UObject* InWorldContextObject, UTexture2D* WebPTexture)
{
	UGameInstance* GameInstance = nullptr;
	if (!GetWorldContextGameInstance(InWorldContextObject, GameInstance))
	{
		return;
	}
	UWebPAnimDisplaySubsystem* AnimDisplaySubsystem = GameInstance->GetSubsystem<UWebPAnimDisplaySubsystem>();
	AnimDisplaySubsystem->ReleaseWebPAnimPicture(WebPTexture);
}

bool UWebPSupportBPLibrary::GetWorldContextGameInstance(const UObject* InWorldContextObject, UGameInstance*& OutGameInstance)
{
	if (!InWorldContextObject)
	{
		UE_LOG(WebPSupportLog, Warning, TEXT("[%s]: InWorldContextObject is null"), *FString(__FUNCTION__));
		return false;
	}
	UWorld* World = InWorldContextObject->GetWorld();
	if (!World)
	{
		UE_LOG(WebPSupportLog, Warning, TEXT("[%s]: InWorldContextObject world is null"), *FString(__FUNCTION__));
		return false;
	}
	OutGameInstance = World->GetGameInstance();
	return true;
}
