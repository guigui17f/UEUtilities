// Copyright 2025 guigui17f. All Rights Reserved.

#include "Subsystems/WebPScreenRecordSubsystem.h"
#include "Core/WebPSupportCore.h"
#include "Log/WebPSupportLog.h"

void UWebPScreenRecordSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GenerateEndCallback.BindUObject(this, &UWebPScreenRecordSubsystem::FinishGenerationProcess);
}

void UWebPScreenRecordSubsystem::Deinitialize()
{
	Super::Deinitialize();
	GenerateEndCallback.Unbind();
	ResetRecordScreen();
}

void UWebPScreenRecordSubsystem::Tick(float DeltaTime)
{
	if (ProcessState == EWebPRecordScreenProcessState::Recording)
	{
		TakeScreenshot(DeltaTime);
	}
}

bool UWebPScreenRecordSubsystem::IsTickable() const
{
	return !IsTemplate();
}

TStatId UWebPScreenRecordSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UWebPScreenRecordSubsystem, STATGROUP_Tickables);
}

bool UWebPScreenRecordSubsystem::StartRecordScreen(const FString& InSavePath)
{
	FVector2D ViewportSize;
	if (WebPSupportCore::GetViewportSize(GetWorld(), ViewportSize))
	{
		FIntVector4 RecordArea(0, 0, ViewportSize.X - 1, ViewportSize.Y - 1);
		return StartRecordScreen(InSavePath, RecordArea);
	}
	else
	{
		UE_LOG(WebPSupportLog, Warning, TEXT("[%s]: get viewport size failed"), *FString(__FUNCTION__));
		return false;
	}
}

bool UWebPScreenRecordSubsystem::StartRecordScreen(const FString& InSavePath, FIntVector4 RecordArea)
{
	if (ProcessState != EWebPRecordScreenProcessState::None)
	{
		UE_LOG(WebPSupportLog, Warning, TEXT("[%s]: ProcessState is not None, current is %d"), *FString(__FUNCTION__), static_cast<uint8>(ProcessState));
		return false;
	}
	FVector2D ViewportSize;
	if (!WebPSupportCore::GetViewportSize(GetWorld(), ViewportSize))
	{
		UE_LOG(WebPSupportLog, Warning, TEXT("[%s]: get viewport size failed"), *FString(__FUNCTION__));
		return false;
	}
	if (RecordArea.X < 0 || RecordArea.Y < 0 || RecordArea.Z > ViewportSize.X - 1 || RecordArea.W > ViewportSize.Y - 1 || RecordArea.X >= RecordArea.Z || RecordArea.Y >= RecordArea.W)
	{
		UE_LOG(WebPSupportLog, Warning, TEXT("[%s]: viewport size is illegal"), *FString(__FUNCTION__));
		return false;
	}

	ResetRecordScreen();
	ProcessState = EWebPRecordScreenProcessState::Recording;
	ScreenRecordSavePath = InSavePath;
	RecordScreenArea = RecordArea;

	FlushRenderingCommands();
	ScreenHandle = UGameViewportClient::OnScreenshotCaptured().AddUObject(this, &UWebPScreenRecordSubsystem::AddScreenRecordData);

	return true;
}

void UWebPScreenRecordSubsystem::TakeScreenshot(float DeltaTime)
{
	// ensure last request has received the callback
	if (ScreenshotRequestCount != ScreenshotCallbackCount)
	{
		return;
	}
	if (WebPTimestamps.Num() == 0)
	{
		WebPTimestamps.Emplace(0);
	}
	else
	{
		WebPTimestamps.Emplace(DeltaTime * 1000);
	}
	FScreenshotRequest::RequestScreenshot(false);
	++ScreenshotRequestCount;
}

void UWebPScreenRecordSubsystem::EndRecordScreen(const FOnGenerateWebPScreenRecordEndBP& InEndCallback)
{
	if (ProcessState != EWebPRecordScreenProcessState::Recording)
	{
		InEndCallback.ExecuteIfBound(false);
		UE_LOG(WebPSupportLog, Warning, TEXT("[%s]: ProcessState is not Recording, current is %d"), *FString(__FUNCTION__), static_cast<uint8>(ProcessState));
		return;
	}

	ProcessState = EWebPRecordScreenProcessState::Generating;
	GenerateEndBPCallback = InEndCallback;

	TWeakObjectPtr WeakThis(this);
	AsyncTask(ENamedThreads::AnyThread, [WeakThis]()
	{
		if (!WeakThis.IsValid())
		{
			return;
		}
		UWebPScreenRecordSubsystem* This = WeakThis.Get();
		// wait for the color data fill work
		for (int32 i = 0; i < 10; ++i)
		{
			FPlatformProcess::Sleep(0.2f);
			if (!WeakThis.IsValid())
			{
				return;
			}
			if (This->WebPColors.Num() == This->ScreenshotRequestCount)
			{
				break;
			}
		}

		int32 PictureWidth = This->RecordScreenArea.Z - This->RecordScreenArea.X + 1;
		int32 PictureHeight = This->RecordScreenArea.W - This->RecordScreenArea.Y + 1;
		FScopeLock RecordScreenLock(&This->RecordScreenMutex);
		if (!WeakThis.IsValid())
		{
			return;
		}

		bool bSuccess = WebPSupportCore::GenerateAnimPicture(This->ScreenRecordSavePath, This->WebPColors, This->WebPTimestamps, PictureWidth, PictureHeight);
		UE_LOG(WebPSupportLog, Display, TEXT("generate animate picture result: %s"), bSuccess ? TEXT("Success") : TEXT("Failed"));

		AsyncTask(ENamedThreads::GameThread, [WeakThis, bSuccess]()
		{
			if (!WeakThis.IsValid())
			{
				return;
			}
			WeakThis->GenerateEndCallback.ExecuteIfBound(bSuccess);
		});
	});
}

void UWebPScreenRecordSubsystem::ResetRecordScreen()
{
	ProcessState = EWebPRecordScreenProcessState::None;
	ScreenshotRequestCount = 0;
	ScreenshotCallbackCount = 0;
	ScreenRecordSavePath = FString();
	WebPColors.Empty();
	WebPTimestamps.Empty();
	if (ScreenHandle.IsValid())
	{
		UGameViewportClient::OnScreenshotCaptured().Remove(ScreenHandle);
		ScreenHandle.Reset();
	}
}

void UWebPScreenRecordSubsystem::AddScreenRecordData(int32 Width, int32 Height, const TArray<FColor>& InColors)
{
	++ScreenshotCallbackCount;
	TWeakObjectPtr WeakThis(this);
	AsyncTask(ENamedThreads::AnyThread, [WeakThis, Width, Height, InColors]()
	{
		if (!WeakThis.IsValid())
		{
			return;
		}
		UWebPScreenRecordSubsystem* This = WeakThis.Get();
		FScopeLock RecordScreenLock(&This->RecordScreenMutex);
		if (!WeakThis.IsValid())
		{
			return;
		}

		TArray<FColor> RecordColors;
		for (int32 Y = This->RecordScreenArea.Y + 1; Y <= This->RecordScreenArea.W + 1 && Y <= Height; ++Y)
		{
			for (int32 X = This->RecordScreenArea.X + 1; X <= This->RecordScreenArea.Z + 1 && Y <= Width; ++X)
			{
				RecordColors.Emplace(InColors[(Y - 1) * Width + X - 1]);
			}
		}

		int32 PictureWidth = This->RecordScreenArea.Z - This->RecordScreenArea.X + 1;
		int32 PictureHeight = This->RecordScreenArea.W - This->RecordScreenArea.Y + 1;
		if (RecordColors.Num() == PictureWidth * PictureHeight)
		{
			This->WebPColors.Emplace(RecordColors);
		}
		else
		{
			UE_LOG(WebPSupportLog, Warning, TEXT("[%s]: RecordColors size error, RecordColors size is %d, expect size is %d, current index is %d"), *FString(__FUNCTION__), RecordColors.Num(), PictureWidth * PictureHeight, This->WebPColors.Num());
			if (This->WebPColors.Num() > 0)
			{
				// add a copy of the recent data to make sure the color array length is same with the timestamps array
				This->WebPColors.Emplace(This->WebPColors.Last());
			}
		}
	});
}

void UWebPScreenRecordSubsystem::FinishGenerationProcess(bool bSuccess)
{
	ResetRecordScreen();
	GenerateEndBPCallback.ExecuteIfBound(bSuccess);
}
