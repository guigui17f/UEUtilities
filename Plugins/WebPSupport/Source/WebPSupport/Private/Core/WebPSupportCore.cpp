// Copyright 2025 guigui17f. All Rights Reserved.

#include "Core/WebPSupportCore.h"
#include "Core/WebPSupportLib.h"
#include "Log/WebPSupportLog.h"

bool WebPSupportCore::GenerateStaticPicture(const FString& InSavePath, const TArray<FColor>& InColorData, int32 Width, int32 Height, float QualityFactor)
{
	if (!CheckSavePathLegality(InSavePath))
	{
		return false;
	}
	if (InColorData.Num() != Width * Height)
	{
		return false;
	}

	// notice: only support ascii path
	const char* OutputPath = TCHAR_TO_ANSI(*InSavePath);

	uint8_t* RGBAData = new uint8_t[InColorData.Num() * 4 + 1];
	memset(RGBAData, 0, InColorData.Num() * 4 + 1);
	int i = 0;
	for (auto& Color : InColorData)
	{
		RGBAData[i++] = Color.R;
		RGBAData[i++] = Color.G;
		RGBAData[i++] = Color.B;
		RGBAData[i++] = Color.A;
	}

	QualityFactor = FMath::Clamp(QualityFactor, 0.f, 100.f);

	bool bResult = WebPSupportLib::GeneratePictureByRGBA(OutputPath, RGBAData, Width, Height, QualityFactor);

	delete[] RGBAData;
	RGBAData = nullptr;

	return bResult;
}

bool WebPSupportCore::GenerateAnimPicture(const FString& InSavePath, const TArray<TArray<FColor>>& InColorDataArray, const TArray<int32>& InTimestampsMs, int32 Width, int32 Height, float QualityFactor)
{
	if (!CheckSavePathLegality(InSavePath))
	{
		return false;
	}
	if (InColorDataArray.Num() != InTimestampsMs.Num())
	{
		return false;
	}
	int PictureSize = Width * Height;
	for (auto& ColorData : InColorDataArray)
	{
		if (ColorData.Num() != PictureSize)
		{
			return false;
		}
	}

	// notice: only support ascii path
	const char* OutputPath = TCHAR_TO_ANSI(*InSavePath);

	std::vector<const uint8_t*> RGBADataArray;
	for (auto& ColorData : InColorDataArray)
	{
		uint8_t* RGBAData = new uint8_t[ColorData.Num() * 4 + 1];
		memset(RGBAData, 0, ColorData.Num() * 4 + 1);
		int i = 0;
		for (auto& Color : ColorData)
		{
			RGBAData[i++] = Color.R;
			RGBAData[i++] = Color.G;
			RGBAData[i++] = Color.B;
			RGBAData[i++] = Color.A;
		}
		RGBADataArray.push_back(RGBAData);
	}

	std::vector<int> Timestamps;
	for (auto& Timestamp : InTimestampsMs)
	{
		Timestamps.push_back(Timestamp);
	}

	QualityFactor = FMath::Clamp(QualityFactor, 0.f, 100.f);

	bool bResult = WebPSupportLib::GenerateAnimByRGBA(OutputPath, RGBADataArray, Timestamps, Width, Height, QualityFactor);

	for (auto& RGBAData : RGBADataArray)
	{
		delete[] RGBAData;
		RGBAData = nullptr;
	}

	return bResult;
}

bool WebPSupportCore::CheckSavePathLegality(const FString& InSavePath)
{
	FString Directory = FPaths::GetPath(InSavePath);
	FString Extension = FPaths::GetExtension(InSavePath);
	if (FPaths::DirectoryExists(Directory) && Extension == TEXT("webp"))
	{
		return true;
	}
	else
	{
		UE_LOG(WebPSupportLog, Warning, TEXT("[%s]: InSavePath is illegal"), *FString(__FUNCTION__));
		return false;
	}
}

bool WebPSupportCore::GetViewportSize(const UWorld* InWorld, FVector2D& OutViewportSize)
{
	if (!InWorld)
	{
		return false;
	}
	UGameViewportClient* ViewportClient = InWorld->GetGameViewport();
	if (!ViewportClient)
	{
		return false;
	}
	OutViewportSize = FVector2D::ZeroVector;
	ViewportClient->GetViewportSize(OutViewportSize);
	return true;
}

bool WebPSupportCore::LoadStaticPicture(const FString& InFilePath, TArray<FColor>& OutColorData, int32& OutWidth, int32& OutHeight)
{
	if (!CheckLoadPathLegality(InFilePath))
	{
		return false;
	}

	// notice: only support ascii path
	const char* LoadFileData = TCHAR_TO_ANSI(*InFilePath);

	uint8_t* RGBAData = nullptr;
	bool bSuccess = WebPSupportLib::LoadPictureByRGBA(LoadFileData, RGBAData, OutWidth, OutHeight);
	if (!bSuccess || !RGBAData)
	{
		return false;
	}

	OutColorData.Empty();
	OutColorData.AddDefaulted(OutWidth* OutHeight);
	FMemory::Memcpy(OutColorData.GetData(), RGBAData, OutWidth* OutHeight*4);
	free(RGBAData);
	RGBAData = nullptr;

	return true;
}

bool WebPSupportCore::LoadAnimPicture(const FString& InFilePath, TArray<TArray<FColor>>& OutColorDataArray, TArray<int32>& OutTimestampsMs, int32& OutWidth, int32& OutHeight)
{
	if (!CheckLoadPathLegality(InFilePath))
	{
		return false;
	}

	// notice: only support ascii path
	const char* LoadFileData = TCHAR_TO_ANSI(*InFilePath);
	OutColorDataArray.Empty();
	OutTimestampsMs.Empty();
	std::vector<uint8_t*> RGBADataArray;
	std::vector<int> Timestamps;

	bool bSuccess = WebPSupportLib::LoadAnimByRGBA(LoadFileData, RGBADataArray, Timestamps, OutWidth, OutHeight);
	if (!bSuccess)
	{
		return false;
	}
	
	int32 PixelSize = OutWidth * OutHeight;
	int32 BinarySize = PixelSize * 4;
	OutColorDataArray.AddDefaulted(RGBADataArray.size());
	for (int32 i = 0; i < RGBADataArray.size(); ++i)
	{
		TArray<FColor>& ColorData = OutColorDataArray[i];
		ColorData.AddDefaulted(PixelSize);
		FMemory::Memcpy(ColorData.GetData(), RGBADataArray[i], BinarySize);
		OutTimestampsMs.Emplace(Timestamps[i]);
		free(RGBADataArray[i]);
	}

	return true;
}

bool WebPSupportCore::CheckIsAnimPicture(const FString& InFilePath, bool& OutIsAnim)
{
	if (!CheckLoadPathLegality(InFilePath))
	{
		return false;
	}

	// notice: only support ascii path
	const char* LoadFileData = TCHAR_TO_ANSI(*InFilePath);
	return WebPSupportLib::CheckIsAnimPicture(LoadFileData, OutIsAnim);
}

bool WebPSupportCore::CheckLoadPathLegality(const FString& InFilePath)
{
	FString Extension = FPaths::GetExtension(InFilePath);
	if (FPaths::FileExists(InFilePath) && Extension == TEXT("webp"))
	{
		return true;
	}
	else
	{
		UE_LOG(WebPSupportLog, Warning, TEXT("[%s]: InFilePath doesn't exist or is illegal"), *FString(__FUNCTION__));
		return false;
	}
}
