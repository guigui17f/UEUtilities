// Copyright 2025 guigui17f. All Rights Reserved.

#include "Subsystems/WebPScreenshotSubsystem.h"
#include "Core/WebPSupportCore.h"

void UWebPScreenshotSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GenerateEndCallback.BindUObject(this, &UWebPScreenshotSubsystem::FinishGenerationProcess);
}

void UWebPScreenshotSubsystem::Deinitialize()
{
	GenerateEndCallback.Unbind();
	Super::Deinitialize();
}

void UWebPScreenshotSubsystem::GenerateScreenShotAsync(const FString& InSavePath, const FOnGenerateWebPScreenshotEndBP& InEndCallback)
{
	if (bInProcessing)
	{
		InEndCallback.ExecuteIfBound(false);
		return;
	}

	bInProcessing = true;
	ScreenshotSavePath = InSavePath;
	GenerateEndBPCallback = InEndCallback;

	FlushRenderingCommands();
	ScreenshotHandle = UGameViewportClient::OnScreenshotCaptured().AddUObject(this, &UWebPScreenshotSubsystem::GenerateWebPPicture);
	FScreenshotRequest::RequestScreenshot(false);
}

void UWebPScreenshotSubsystem::GenerateWebPPicture(int32 Width, int32 Height, const TArray<FColor>& InColors)
{
	if (ScreenshotHandle.IsValid())
	{
		UGameViewportClient::OnScreenshotCaptured().Remove(ScreenshotHandle);
		ScreenshotHandle.Reset();
	}

	TWeakObjectPtr WeakThis(this);
	AsyncTask(ENamedThreads::AnyThread, [WeakThis, Width, Height, InColors]()
	{
		if (!WeakThis.IsValid())
		{
			return;
		}
		bool bSuccess = WebPSupportCore::GenerateStaticPicture(WeakThis->ScreenshotSavePath, InColors, Width, Height);
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

void UWebPScreenshotSubsystem::FinishGenerationProcess(bool bSuccess)
{
	bInProcessing = false;
	ScreenshotSavePath = FString();
	GenerateEndBPCallback.ExecuteIfBound(bSuccess);
}
