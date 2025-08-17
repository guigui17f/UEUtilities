// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WebPScreenshotSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGenerateWebPScreenshotEndBP, bool, bSuccess);

UCLASS()
class WEBPSUPPORT_API UWebPScreenshotSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	friend class UWebPSupportBPLibrary;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

protected:
	void GenerateScreenShotAsync(const FString& InSavePath, const FOnGenerateWebPScreenshotEndBP& InEndCallback);
	
	UFUNCTION()
	void GenerateWebPPicture(int32 Width, int32 Height, const TArray<FColor>& InColors);

	UFUNCTION()
	void FinishGenerationProcess(bool bSuccess);

protected:
	DECLARE_DELEGATE_OneParam(FOnGenerateScreenshotEnd, bool)

	FString ScreenshotSavePath;

	FDelegateHandle ScreenshotHandle;

	bool bInProcessing = false;

	FOnGenerateScreenshotEnd GenerateEndCallback;

	FOnGenerateWebPScreenshotEndBP GenerateEndBPCallback;
};
