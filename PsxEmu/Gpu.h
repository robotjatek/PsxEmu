#pragma once
#include <cstdint>
#include "RendererGL.h"
#include "GPUData.h"

#define VRAM_SIZE 1048510 //1 MB of VRAM
//TODO: GPU timer
class Gpu
{
private:
	RendererGL renderer;
	uint8_t* const vram;
	uint32_t commandBuffer[16]; //64 byte (16 word) command buffer
	uint8_t currentCommand;
	bool inCommand;
	uint32_t commandState;
	uint32_t remainingWords;
	PolygonData currentPolygon;

	void EnterCommandProcessing(uint8_t command);

	void GraduatedPolygon(uint8_t polyCount, uint32_t data);

	void TexturedPolygon(uint8_t polyCount, uint32_t data);

public:
	Gpu();
	virtual ~Gpu();
	void SendGP0Command(uint32_t data); //0x1f801810 write
	uint32_t GetGPURead(); //0x1f801810 read
	void SendGP1Command(uint32_t data); //0x1f801814 write
	uint32_t GetGPUStatus() const; //0x1f801814 read
};

