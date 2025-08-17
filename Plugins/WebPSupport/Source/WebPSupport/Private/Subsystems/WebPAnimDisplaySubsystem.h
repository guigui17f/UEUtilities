// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/WebPDisplayProcessState.h"
#include "WebPAnimDisplaySubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_FiveParams(FOnWebPAnimPictureLoadEndBP, bool, bSuccess, int64, ProcessId, UTexture2D*, OutTexture, int32, OutWidth, int32, OutHeight);

UCLASS()
class UWebPAnimDisplaySubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

	friend class UWebPSupportBPLibrary;

public:
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;

	virtual bool IsTickable() const override;

	virtual TStatId GetStatId() const override;

protected:
	void LoadWebPAnimPicture(const FString& InFilePath, const FOnWebPAnimPictureLoadEndBP& InLoadEndCallback);

	void ReleaseWebPAnimPicture(int64 ProcessId);

	void ReleaseWebPAnimPicture(UTexture2D* WebPTexture);

	void TryCreateLoadedPicture(bool bSuccess, int64 ProcessId, int32 Width, int32 Height);

protected:
	UPROPERTY()
	TMap<int64, UTexture2D*> WebPTextures;

	TMap<int64, EWebPDisplayProcessState> ProcessStates;

	TMap<int64, FOnWebPAnimPictureLoadEndBP> LoadEndCallbacks;

	TMap<int64, TArray<TArray<FColor>>> WebPColorDataArrayMap;

	TMap<int64, TArray<int32>> WebPMillisecondsMap;

	TMap<int64, int32> WebPPictureBinarySizes;

	TMap<int64, int32> DisplayIndexes;

	TMap<int64, int32> DisplayMilliseconds;

	TArray<int64> InvalidProcessIds;

	int64 ProcessIdIndex = 0;
};
