// Copyright 2025 guigui17f. All Rights Reserved.

#include "Core/WebPSupportLib.h"
#include "encode.h"
#include "decode.h"
#include "mux.h"
#include "demux.h"

bool WebPSupportLib::GeneratePictureByRGBA(const char* InSavePath, const uint8_t* InRGBAData, int Width, int Height, float QualityFactor)
{
	if (QualityFactor < 0.f)
	{
		QualityFactor = 0.f;
	}
	else if (QualityFactor > 100.f)
	{
		QualityFactor = 100.f;
	}

	uint8_t* OutputData = nullptr;
	size_t Size = WebPEncodeRGBA(InRGBAData, Width, Height, Width * 4, QualityFactor, &OutputData);
	if (Size == 0)
	{
		return false;
	}

	FILE* FileDescriptor;
	errno_t Error = fopen_s(&FileDescriptor, InSavePath, "wb");

	if (Error == 0 && FileDescriptor != nullptr)
	{
		fwrite(OutputData, Size, 1, FileDescriptor);
		fclose(FileDescriptor);
		WebPFree(OutputData);
		return true;
	}
	else
	{
		WebPFree(OutputData);
		return false;
	}
}

bool WebPSupportLib::GenerateAnimByRGBA(const char* InSavePath, std::vector<const uint8_t*>& InRGBAData, std::vector<int> InTimestampsMs, int Width, int Height, float QualityFactor)
{
	if (InRGBAData.size() != InTimestampsMs.size())
	{
		return false;
	}
	WebPConfig Config;
	if (!WebPConfigInit(&Config))
	{
		return false;
	}
	WebPAnimEncoderOptions EncodeOptions;
	if (!WebPAnimEncoderOptionsInit(&EncodeOptions))
	{
		return false;
	}

	if (QualityFactor < 0.f)
	{
		QualityFactor = 0.f;
	}
	else if (QualityFactor > 100.f)
	{
		QualityFactor = 100.f;
	}
	Config.lossless = 0;
	Config.method = 6;
	Config.quality = QualityFactor;
	EncodeOptions.kmin = 0;
	EncodeOptions.kmax = 1;

	WebPAnimEncoder* Encoder = WebPAnimEncoderNew(Width, Height, &EncodeOptions);

	float Timestamp = 0.f;
	std::vector<const uint8_t*>::iterator RGBAItem = InRGBAData.begin();
	std::vector<int>::iterator TimestampItem = InTimestampsMs.begin();
	for (; RGBAItem != InRGBAData.end(); ++RGBAItem, ++TimestampItem)
	{
		WebPPicture Picture;
		if (!WebPPictureInit(&Picture))
		{
			break;
		}
		Picture.use_argb = 1;
		Picture.width = Width;
		Picture.height = Height;
		Picture.argb_stride = Width * 4;

		WebPPictureImportRGBA(&Picture, *RGBAItem, Width * 4);
		Timestamp += *TimestampItem;
		WebPAnimEncoderAdd(Encoder, &Picture, Timestamp, &Config);
		WebPPictureFree(&Picture);
	}
	WebPAnimEncoderAdd(Encoder, nullptr, Timestamp, nullptr);

	WebPData OutputData;
	WebPAnimEncoderAssemble(Encoder, &OutputData);
	WebPAnimEncoderDelete(Encoder);

	FILE* FileDescriptor;
	errno_t Error = fopen_s(&FileDescriptor, InSavePath, "wb");

	if (Error == 0 && FileDescriptor != nullptr)
	{
		fwrite(OutputData.bytes, OutputData.size, 1, FileDescriptor);
		fclose(FileDescriptor);
		WebPDataClear(&OutputData);
		return true;
	}
	else
	{
		WebPDataClear(&OutputData);
		return false;
	}
}

bool WebPSupportLib::CheckIsAnimPicture(const char* InFilePath, bool& OutIsAnim)
{
	FILE* FileDescriptor;
	errno_t Error = fopen_s(&FileDescriptor, InFilePath, "rb");
	if (Error != 0 || FileDescriptor == nullptr)
	{
		return false;
	}

	fseek(FileDescriptor, 0, SEEK_END);
	size_t FileLength = ftell(FileDescriptor);
	void* FileData = WebPMalloc(FileLength);
	memset(FileData, 0, FileLength);
	fseek(FileDescriptor, 0, SEEK_SET);
	fread(FileData, FileLength, 1, FileDescriptor);
	fclose(FileDescriptor);

	WebPData WebPData = {static_cast<uint8_t*>(FileData), FileLength};
	WebPDemuxer* Demuxer = WebPDemux(&WebPData);
	if (!Demuxer)
	{
		WebPDataClear(&WebPData);
		return false;
	}

	uint32_t flags = WebPDemuxGetI(Demuxer, WEBP_FF_FORMAT_FLAGS);
	OutIsAnim = (flags & ANIMATION_FLAG) != 0;

	WebPDemuxDelete(Demuxer);
	WebPDataClear(&WebPData);
	return true;
}

bool WebPSupportLib::LoadPictureByRGBA(const char* InFilePath, uint8_t*& OutRGBAData, int& OutWidth, int& OutHeight)
{
	FILE* FileDescriptor;
	errno_t Error = fopen_s(&FileDescriptor, InFilePath, "rb");
	if (Error != 0 || FileDescriptor == nullptr)
	{
		return false;
	}

	fseek(FileDescriptor, 0, SEEK_END);
	size_t FileLength = ftell(FileDescriptor);
	void* FileData = WebPMalloc(FileLength);
	memset(FileData, 0, FileLength);
	fseek(FileDescriptor, 0, SEEK_SET);
	fread(FileData, FileLength, 1, FileDescriptor);
	fclose(FileDescriptor);

	uint8_t* DecodeData = WebPDecodeRGBA(static_cast<uint8_t*>(FileData), FileLength, &OutWidth, &OutHeight);
	if (!DecodeData)
	{
		WebPFree(FileData);
		return false;
	}
	OutRGBAData = static_cast<uint8_t*>(malloc(OutWidth * OutHeight * 4));
	if (!OutRGBAData)
	{
		WebPFree(DecodeData);
		WebPFree(FileData);
		return false;
	}
	memcpy(OutRGBAData, DecodeData, OutWidth * OutHeight * 4);
	WebPFree(DecodeData);
	WebPFree(FileData);
	return true;
}

bool WebPSupportLib::LoadAnimByRGBA(const char* InFilePath, std::vector<uint8_t*>& OutRGBAData, std::vector<int>& OutTimestampMs, int& OutWidth, int& OutHeight)
{
	FILE* FileDescriptor;
	errno_t Error = fopen_s(&FileDescriptor, InFilePath, "rb");
	if (Error != 0 || FileDescriptor == nullptr)
	{
		return false;
	}

	fseek(FileDescriptor, 0, SEEK_END);
	size_t FileLength = ftell(FileDescriptor);
	void* FileData = WebPMalloc(FileLength);
	memset(FileData, 0, FileLength);
	fseek(FileDescriptor, 0, SEEK_SET);
	fread(FileData, FileLength, 1, FileDescriptor);
	fclose(FileDescriptor);

	WebPData WebPData = {static_cast<uint8_t*>(FileData), FileLength};
	WebPAnimDecoderOptions DecoderOptions;
	if (!WebPAnimDecoderOptionsInit(&DecoderOptions))
	{
		WebPDataClear(&WebPData);
		return false;
	}
	WebPAnimDecoder* Decoder = WebPAnimDecoderNew(&WebPData, &DecoderOptions);
	if (!Decoder)
	{
		WebPDataClear(&WebPData);
		return false;
	}
	WebPAnimInfo WebPInfo;
	if (!WebPAnimDecoderGetInfo(Decoder, &WebPInfo))
	{
		WebPAnimDecoderDelete(Decoder);
		WebPDataClear(&WebPData);
		return false;
	}

	OutWidth = WebPInfo.canvas_width;
	OutHeight = WebPInfo.canvas_height;
	int PictureSize = OutWidth * OutHeight * 4;
	OutRGBAData.clear();
	OutTimestampMs.clear();
	int FrameCount = 0;
	int Timestamp = 0;
	while (WebPAnimDecoderHasMoreFrames(Decoder))
	{
		uint8_t* buffer;
		WebPAnimDecoderGetNext(Decoder, &buffer, &Timestamp);
		++FrameCount;

		void* FrameData = malloc(PictureSize);
		memcpy(FrameData, buffer, PictureSize);

		OutRGBAData.push_back(static_cast<uint8_t*>(FrameData));
		OutTimestampMs.push_back(Timestamp);
	}

	WebPAnimDecoderReset(Decoder);
	WebPAnimDecoderDelete(Decoder);
	WebPDataClear(&WebPData);

	return true;
}
