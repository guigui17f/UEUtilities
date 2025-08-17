// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/WebPRecordScreenProcessState.h"
#include "WebPScreenRecordSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGenerateWebPScreenRecordEndBP, bool, bSuccess);

UCLASS()
class WEBPSUPPORT_API UWebPScreenRecordSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

	friend class UWebPSupportBPLibrary;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;

	virtual bool IsTickable() const override;

	virtual TStatId GetStatId() const override;

protected:
	bool StartRecordScreen(const FString& InSavePath);

	bool StartRecordScreen(const FString& InSavePath, FIntVector4 RecordArea);

	void TakeScreenshot(float DeltaTime);

	void EndRecordScreen(const FOnGenerateWebPScreenRecordEndBP& InEndCallback);

	void ResetRecordScreen();

	UFUNCTION()
	void AddScreenRecordData(int32 Width, int32 Height, const TArray<FColor>& InColors);

	UFUNCTION()
	void FinishGenerationProcess(bool bSuccess);

protected:
	DECLARE_DELEGATE_OneParam(FOnGenerateScreenRecordEnd, bool)

	FString ScreenRecordSavePath;

	FIntVector4 RecordScreenArea;

	EWebPRecordScreenProcessState ProcessState = EWebPRecordScreenProcessState::None;

	FDelegateHandle ScreenHandle;

	TArray<TArray<FColor>> WebPColors;

	TArray<int32> WebPTimestamps;

	FCriticalSection RecordScreenMutex;

	int32 ScreenshotRequestCount;
	
	int32 ScreenshotCallbackCount;

	FOnGenerateScreenRecordEnd GenerateEndCallback;

	FOnGenerateWebPScreenRecordEndBP GenerateEndBPCallback;
};
