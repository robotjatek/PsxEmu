#include "Gpu.h"
#include <iostream>


Gpu::Gpu()
{
	vram = new uint8_t[VRAM_SIZE];
}


Gpu::~Gpu()
{
	delete[] vram;
}

void Gpu::SendGP0Command(uint32_t data)
{
	//printf("%08x\n",data);
}

uint32_t Gpu::GetGPURead()
{
	return uint32_t();
}

void Gpu::SendGP1Command(uint32_t data)
{
}

uint32_t Gpu::GetGPUStatus()
{
	return 0x1c000000;
}
