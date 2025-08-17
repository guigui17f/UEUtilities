// Copyright 2025 guigui17f. All Rights Reserved.


#include "Subsystems/WebPAnimDisplaySubsystem.h"
#include "Core/WebPSupportCore.h"
#include "Log/WebPSupportLog.h"

void UWebPAnimDisplaySubsystem::Deinitialize()
{
	ProcessIdIndex = 0;
	ProcessStates.Empty();
	WebPTextures.Empty();
	LoadEndCallbacks.Empty();
	WebPColorDataArrayMap.Empty();
	WebPMillisecondsMap.Empty();
	WebPPictureBinarySizes.Empty();
	DisplayIndexes.Empty();
	DisplayMilliseconds.Empty();
	InvalidProcessIds.Empty();
	Super::Deinitialize();
}

void UWebPAnimDisplaySubsystem::Tick(float DeltaTime)
{
	DeltaTime *= 1000;
	InvalidProcessIds.Empty();
	for (auto& StateData : ProcessStates)
	{
		if (StateData.Value != EWebPDisplayProcessState::Displaying)
		{
			continue;
		}
		UTexture2D* WebPTexture = WebPTextures.FindRef(StateData.Key);
		if (!IsValid(WebPTexture))
		{
			InvalidProcessIds.Emplace(StateData.Key);
			continue;
		}
		int64 ProcessId = StateData.Key;
		int32& DisplayIndex = DisplayIndexes[ProcessId];
		int32& DisplayMillisecond = DisplayMilliseconds[ProcessId];
		DisplayMillisecond += DeltaTime;
		if (DisplayMillisecond > WebPMillisecondsMap[ProcessId][DisplayIndex])
		{
			++DisplayIndex;
			if (DisplayIndex >= WebPColorDataArrayMap[ProcessId].Num())
			{
				DisplayIndex = 0;
				DisplayMillisecond = 0;
			}
			void* TextureData = WebPTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, WebPColorDataArrayMap[ProcessId][DisplayIndex].GetData(), WebPPictureBinarySizes[ProcessId]);
			WebPTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
			WebPTexture->UpdateResource();
		}
	}
	for (int64 InvalidId : InvalidProcessIds)
	{
		ReleaseWebPAnimPicture(InvalidId);
	}
}

bool UWebPAnimDisplaySubsystem::IsTickable() const
{
	return !IsTemplate();
}

TStatId UWebPAnimDisplaySubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UWebPAnimDisplaySubsystem, STATGROUP_Tickables);
}

void UWebPAnimDisplaySubsystem::LoadWebPAnimPicture(const FString& InFilePath, const FOnWebPAnimPictureLoadEndBP& InLoadEndCallback)
{
	if (!WebPSupportCore::CheckLoadPathLegality(InFilePath))
	{
		InLoadEndCallback.ExecuteIfBound(false, 0, nullptr, 0, 0);
		return;
	}

	int64 ProcessId = ProcessIdIndex++;
	ProcessStates.Emplace(ProcessId, EWebPDisplayProcessState::Loading);
	LoadEndCallbacks.Emplace(ProcessId, InLoadEndCallback);

	TWeakObjectPtr WeakThis(this);
	AsyncTask(ENamedThreads::AnyThread, [WeakThis, ProcessId, InFilePath]()
	{
		if (!WeakThis.IsValid())
		{
			return;
		}
		UWebPAnimDisplaySubsystem* This = WeakThis.Get();
		TArray<TArray<FColor>>& ColorDataArray = This->WebPColorDataArrayMap.Emplace(ProcessId);
		TArray<int32>& Milliseconds = This->WebPMillisecondsMap.Emplace(ProcessId);
		int32 Width;
		int32 Height;

		bool bSuccess = WebPSupportCore::LoadAnimPicture(InFilePath, ColorDataArray, Milliseconds, Width, Height);
		if (!WeakThis.IsValid())
		{
			return;
		}
		if (bSuccess)
		{
			This->WebPPictureBinarySizes.Emplace(ProcessId, Width * Height * 4);
		}

		AsyncTask(ENamedThreads::GameThread, [WeakThis, bSuccess, ProcessId, Width, Height]()
		{
			if (!WeakThis.IsValid())
			{
				return;
			}
			WeakThis->TryCreateLoadedPicture(bSuccess, ProcessId, Width, Height);
		});
	});
}

void UWebPAnimDisplaySubsystem::ReleaseWebPAnimPicture(int64 ProcessId)
{
	WebPTextures.Remove(ProcessId);
	LoadEndCallbacks.Remove(ProcessId);
	ProcessStates.Remove(ProcessId);
	WebPColorDataArrayMap.Remove(ProcessId);
	WebPMillisecondsMap.Remove(ProcessId);
	WebPPictureBinarySizes.Remove(ProcessId);
	DisplayIndexes.Remove(ProcessId);
	DisplayMilliseconds.Remove(ProcessId);
}

void UWebPAnimDisplaySubsystem::ReleaseWebPAnimPicture(UTexture2D* WebPTexture)
{
	UE_LOG(LogTemp, Warning, TEXT("调用时地址: %p"), WebPTexture);
	if (const int64* ProcessId = WebPTextures.FindKey(WebPTexture))
	{
		ReleaseWebPAnimPicture(*ProcessId);
	}
	else
	{
		UE_LOG(WebPSupportLog, Warning, TEXT("[%s]: WebPTexture doesn't exist in WebPTextures map"), *FString(__FUNCTION__));
	}
}

void UWebPAnimDisplaySubsystem::TryCreateLoadedPicture(bool bSuccess, int64 ProcessId, int32 Width, int32 Height)
{
	if (bSuccess)
	{
		ProcessStates[ProcessId] = EWebPDisplayProcessState::Displaying;

		UTexture2D* WebPTexture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);
		void* TextureData = WebPTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(TextureData, WebPColorDataArrayMap[ProcessId][0].GetData(), WebPPictureBinarySizes[ProcessId]);
		WebPTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
		WebPTexture->UpdateResource();

		WebPTextures.Emplace(ProcessId, WebPTexture);
		DisplayIndexes.Emplace(ProcessId, 0);
		DisplayMilliseconds.Emplace(ProcessId, 0);

		LoadEndCallbacks[ProcessId].ExecuteIfBound(true, ProcessId, WebPTexture, Width, Height);

		UE_LOG(LogTemp, Warning, TEXT("创建时地址: %p"), WebPTexture);
	}
	else
	{
		FOnWebPAnimPictureLoadEndBP LoadEndCallback = LoadEndCallbacks[ProcessId];
		ReleaseWebPAnimPicture(ProcessId);
		LoadEndCallback.ExecuteIfBound(false, 0, nullptr, 0, 0);
	}
}
