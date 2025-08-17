// Copyright 2025 guigui17f. All Rights Reserved.


#include "Subsystems/WebPStaticDisplaySubsystem.h"
#include "Core/WebPSupportCore.h"

void UWebPStaticDisplaySubsystem::LoadWebPStaticPicture(const FString& InFilePath, const FOnWebPStaticPictureLoadEndBP& InLoadEndCallback)
{
	if (!WebPSupportCore::CheckLoadPathLegality(InFilePath))
	{
		InLoadEndCallback.ExecuteIfBound(false, nullptr, 0, 0);
		return;
	}
	TWeakObjectPtr WeakThis(this);
	AsyncTask(ENamedThreads::AnyThread, [WeakThis, InFilePath, InLoadEndCallback]()
	{
		if (!WeakThis.IsValid())
		{
			return;
		}
		TArray<FColor> ColorData;
		int32 Width;
		int32 Height;

		bool bSuccess = WebPSupportCore::LoadStaticPicture(InFilePath, ColorData, Width, Height);

		AsyncTask(ENamedThreads::GameThread, [WeakThis, bSuccess, ColorData = MoveTemp(ColorData), Width, Height , InLoadEndCallback]()
		{
			if (!WeakThis.IsValid())
			{
				return;
			}
			WeakThis->TryCreateLoadedPicture(bSuccess, ColorData, Width, Height, InLoadEndCallback);
		});
	});
}

void UWebPStaticDisplaySubsystem::TryCreateLoadedPicture(bool bSuccess, TArray<FColor> ColorData, int32 Width, int32 Height, const FOnWebPStaticPictureLoadEndBP& InLoadEndCallback)
{
	if (bSuccess)
	{
		UTexture2D* WebPTexture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);
		void* TextureData = WebPTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(TextureData, ColorData.GetData(), Width * Height * 4);
		WebPTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
		WebPTexture->UpdateResource();
		InLoadEndCallback.ExecuteIfBound(true, WebPTexture, Width, Height);
	}
	else
	{
		InLoadEndCallback.ExecuteIfBound(false, nullptr, 0, 0);
	}
}
