// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "vector"
#include "CoreMinimal.h"

struct WebPSupportLib
{
	friend class WebPSupportCore;

protected:
	static bool GeneratePictureByRGBA(const char* InSavePath, const uint8_t* InRGBAData, int Width, int Height, float QualityFactor = 100.f);

	static bool GenerateAnimByRGBA(const char* InSavePath, std::vector<const uint8_t*>& InRGBAData, std::vector<int> InTimestampMs, int Width, int Height, float QualityFactor = 100.f);

	static bool CheckIsAnimPicture(const char* InFilePath, bool& OutIsAnim);
	
	static bool LoadPictureByRGBA(const char* InFilePath, uint8_t*& OutRGBAData, int& OutWidth, int& OutHeight);

	static bool LoadAnimByRGBA(const char* InFilePath, std::vector<uint8_t*>& OutRGBAData, std::vector<int>& OutTimestampMs, int& OutWidth, int& OutHeight);
};
