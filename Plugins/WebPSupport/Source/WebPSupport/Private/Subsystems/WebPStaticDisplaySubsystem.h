// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WebPStaticDisplaySubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_FourParams(FOnWebPStaticPictureLoadEndBP, bool, bSuccess, UTexture2D*, OutTexture, int32, OutWidth, int32, OutHeight);

UCLASS()
class UWebPStaticDisplaySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	friend class UWebPSupportBPLibrary;

protected:
	void LoadWebPStaticPicture(const FString& InFilePath, const FOnWebPStaticPictureLoadEndBP& InLoadEndCallback);

	void TryCreateLoadedPicture(bool bSuccess, TArray<FColor> ColorData, int32 Width, int32 Height, const FOnWebPStaticPictureLoadEndBP& InLoadEndCallback);
};
