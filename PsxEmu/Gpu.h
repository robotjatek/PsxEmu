#pragma once
#include <cstdint>
#include "RendererGL.h"
#include "GPUData.h"
#include <functional>

#define VRAM_SIZE 1048510 //1 MB of VRAM
//TODO: GPU timer
class Gpu
{
private:
	struct VRAMAccess
	{
		uint16_t x, y;
		uint16_t xSize, ySize;
		uint16_t currentX;
		uint16_t currentY;
	};

	VRAMAccess vramAccess;
	RendererGL* renderer;
	uint8_t* const vram;
	uint16_t* vramPointer;
	uint32_t commandBuffer[16]; //64 byte (16 word) command buffer
	uint8_t currentCommand;
	bool inCommand;
	uint32_t commandState;
	uint32_t remainingWords;
	PolygonData currentPolygon;

	void EnterCommandProcessing(uint8_t command);
	void GraduatedPolygon(const uint8_t polyCount, const uint32_t data);
	void ExtractColorData(const uint32_t data, const uint8_t polyCount);
	void TexturedPolygon(const uint8_t polyCount, const uint32_t data);
	void MonochromePolygon(const uint8_t polyCount, const uint32_t data);

	void WriteVRAM(uint32_t data);
	void SetVRAMAccessVariables(uint32_t data);
	void IncrementVramAccessHelpers();
	uint32_t CalculateFrambufferPixelId();

public:
	Gpu(RendererGL* renderer);
	virtual ~Gpu();
	void SendGP0Command(const uint32_t data); //0x1f801810 write
	uint32_t GetGPURead(); //0x1f801810 read
	void SendGP1Command(const uint32_t data); //0x1f801814 write
	uint32_t GetGPUStatus() const; //0x1f801814 read
	uint16_t* getVram();
};

