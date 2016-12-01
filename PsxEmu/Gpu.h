#pragma once
#include <cstdint>

#define VRAM_SIZE 1048510 //1 MB of VRAM
//TODO: GPU timer
class Gpu
{
private:
	uint8_t* const vram;
	uint32_t commandBuffer[16]; //64 byte (16 word) command buffer

public:
	Gpu();
	virtual ~Gpu();
	void SendGP0Command(uint32_t data) const; //0x1f801810 write
	uint32_t GetGPURead() const; //0x1f801810 read
	void SendGP1Command(uint32_t data); //0x1f801814 write
	uint32_t GetGPUStatus() const; //0x1f801814 read
};

