#include "Gpu.h"
#include <plog\Log.h>

void Gpu::EnterCommandProcessing(uint8_t command)
{
	this->inCommand = true;
	this->commandState = 0;
	this->currentCommand = command;
}

void Gpu::GraduatedPolygon(uint8_t polyCount, uint32_t data)
{
	int vertexId = commandState / 2;
	if (commandState % 2 == 0)
	{
		currentPolygon.vertices[vertexId].r = (data & 0xff);
		currentPolygon.vertices[vertexId].g = ((data & 0xff00) >> 8);
		currentPolygon.vertices[vertexId].b = (data >> 16);
	}
	else
	{
		currentPolygon.vertices[vertexId].x = data & 0xFFFF;
		currentPolygon.vertices[vertexId].y = data >> 16;
	}

	commandState++;
	if (commandState == polyCount * 2)
	{
		this->inCommand = false;
		this->commandState = 0;
		renderer->PushPolygons(currentPolygon, polyCount);
	}
}

void Gpu::ExtractColorData(const uint32_t data, const uint8_t polyCount)
{
	uint32_t color = data & 0xFFFFFF;
	for (int i = 0; i < polyCount; i++)
	{
		currentPolygon.vertices[i].r = color & 0xff;
		currentPolygon.vertices[i].g = (color & 0xff00) >> 8;
		currentPolygon.vertices[i].b = color >> 16;
	}
}

void Gpu::TexturedPolygon(uint8_t polyCount, uint32_t data)
{
	static uint8_t vertexId = 0;
	if (commandState == 0)
	{
		ExtractColorData(data, polyCount);
	}
	else if (commandState % 2 == 1)
	{
		uint16_t x, y;
		x = data & 0xFFFF;
		y = data >> 16;
		currentPolygon.vertices[vertexId].x = x;
		currentPolygon.vertices[vertexId].y = y;
	}
	else
	{
		uint8_t u, v;
		u = data & 0xff;
		v = (data & 0xff00) >> 8;
		currentPolygon.vertices[vertexId].u = u;
		currentPolygon.vertices[vertexId].v = v;

		if (commandState == 2)
		{
			currentPolygon.ColorLookupTableIndex = data >> 16;
		}
		else if (commandState == 4)
		{
			currentPolygon.TexturePageIndex = data >> 16;
		}
		vertexId++;
	}

	commandState++;
	if (commandState == polyCount * 2 + 1)
	{
		this->inCommand = false;
		this->commandState = 0;
		vertexId = 0;
		renderer->PushPolygons(currentPolygon, 4);
	}
}

Gpu::Gpu(RendererGL* renderer): vram(new uint8_t[VRAM_SIZE])
{
	this->vramPointer = reinterpret_cast<uint16_t*>(vram);
	this->inCommand = false;
	this->commandState = 0;
	this->remainingWords = 0;
	this->renderer = renderer;
}


Gpu::~Gpu()
{
	delete renderer;
	delete[] vram;
}

void Gpu::MonochromePolygon(const uint8_t polyCount, const uint32_t data)
{
	if (commandState == 0)
	{
		ExtractColorData(data, polyCount);
	}
	else
	{
		currentPolygon.vertices[commandState].x = data & 0xFFFF;
		currentPolygon.vertices[commandState].y = data >> 16;
	}

	commandState++;
	if (this->commandState == 4)
	{
		this->inCommand = false;
		this->commandState = 0;
	}
}

void Gpu::WriteVRAM(uint32_t data)
{
	uint32_t frameBufferPixelId = CalculateFrambufferPixelId();

	uint16_t pixel1 = data >> 16;
	uint16_t pixel0 = data & 0xffff;
	this->vramPointer[frameBufferPixelId] = pixel0;
	this->vramPointer[frameBufferPixelId + 1] = pixel1;

	IncrementVramAccessHelpers();

	this->remainingWords--;
	if (remainingWords == 0)
	{
		this->inCommand = false;
		this->commandState = 0;
		vramAccess.x = 0;
		vramAccess.y = 0;
		vramAccess.xSize = 0;
		vramAccess.ySize = 0;
	}
}

void Gpu::IncrementVramAccessHelpers()
{
	vramAccess.currentX += 2;
	if (vramAccess.currentX >= vramAccess.x + vramAccess.xSize)
	{
		vramAccess.currentX = vramAccess.x;
		vramAccess.currentY++;
	}
}

uint32_t Gpu::CalculateFrambufferPixelId()
{
	return vramAccess.currentY * 1024 + vramAccess.currentX;
}

void Gpu::SendGP0Command(uint32_t data)
{
	uint8_t command = (uint8_t)((data & 0xFF000000) >> 24);
	
	if (!inCommand)
	{
		switch (command)
		{
		case 0x00:
			LOG_VERBOSE << "GP0 NOP";
			break;
		case 0x01:
			LOG_INFO << "Implement GP0 CLEAR CACHE";
			//TODO: implement GP0(0x01) - clear cache
			break;
		case 0x28:
		{
			this->EnterCommandProcessing(command);
			MonochromePolygon(4, data);
			break;
		}
		case 0x2c:
		{
			this->EnterCommandProcessing(command);
			this->TexturedPolygon(4, data);
			break;
		}
		case 0x30:
		{
			this->EnterCommandProcessing(command);
			this->GraduatedPolygon(3, data);
			break;
		}
		case 0x38:
		{
			this->EnterCommandProcessing(command);
			GraduatedPolygon(4, data);
			break;
		}
		case 0xa0:
			this->EnterCommandProcessing(command);
			break;
		case 0xc0:
		{
			this->EnterCommandProcessing(command);
			break;
		}
		case 0xe1:
			// TODO: GP0(E1h) - Draw Mode setting (aka "Texpage")
			LOG_WARNING << "Implement GP0(E1h)";
			break;
		case 0xe2:
			// TODO:  GP0(E2h) - Texture Window setting
			LOG_WARNING << "Implement GP0(E2h)";
			break;
		case 0xe3:
			// TODO: GP0(E3h) - Set Drawing Area top left (X1,Y1)
			LOG_WARNING << "Implement GP0(E3h)";
			break;
		case 0xe4:
			// TODO:  GP0(E4h) - Set Drawing Area bottom right (X2,Y2)
			LOG_WARNING << "Implement GP0(E4h)";
			break;
		case 0xe5:
		{
			int16_t offsetX = data & 0x7ff;
			int16_t offsetY = (data & 0x3ff800) >> 11;
			renderer->UpdateDrawingOffset(offsetX, offsetY);
			//XXX: based on noca$h specs. "Everything you wanted to ask..." says only bits 13..11 are the Y offset

			renderer->SwapBuffers(); //TODO: remove this hack
			break;
		}
		case 0xe6:
			//TODO: GP0(E6h) - Mask Bit Setting
			LOG_WARNING << "Implement GP0(E6h)";
			break;
		default:
			LOG_ERROR <<"NOT IMPLEMENTED GP0 COMMAND!";
			LOG_ERROR << "GP0: %08x " << data;
			break;
		}
	}
	else
	{
		switch (currentCommand)
		{
		case 0x28:
		{
			MonochromePolygon(4, data);
			break;
		}
		case 0x2c:
		{
			TexturedPolygon(4, data);
			break;
		}
		case 0x30:
		{
			GraduatedPolygon(3, data);
			break;
		}
		case 0x38:
		{
			GraduatedPolygon(4, data);
			break;
		}
		case 0xa0:
		{
			SetVRAMAccessVariables(data);
			if (commandState > 1)
			{
				WriteVRAM(data);
			}
			commandState++;
			break;
		}
		case 0xc0:
		{
			SetVRAMAccessVariables(data);
			commandState++;
			break;
		}
		default:
			LOG_ERROR << "Something went wrong: Hit default while processing a GPU command";
			break;
		}
	}
}

void Gpu::SetVRAMAccessVariables(uint32_t data)
{
	if (commandState == 0)
	{
		vramAccess.x = data & 0x0000FFFF;
		vramAccess.y = ((data & 0xFFFF0000) >> 16);

		vramAccess.currentX = vramAccess.x;
		vramAccess.currentY = vramAccess.y;

	}
	else if (commandState == 1)
	{
		vramAccess.xSize = data & 0x0000FFFF;
		vramAccess.ySize = ((data & 0xFFFF0000) >> 16);
		uint32_t imagesize = vramAccess.xSize * vramAccess.ySize;
		imagesize = (imagesize + 1) & 0xFFFFFFFE;
		this->remainingWords = imagesize / 2;
	}
}

uint32_t Gpu::GetGPURead()
{
	uint32_t returnValue = 0;
	if (inCommand && currentCommand == 0xc0)
	{
		uint32_t frameBufferPixelId = CalculateFrambufferPixelId();

		uint16_t pixel1 = this->vramPointer[frameBufferPixelId];
		uint16_t pixel0 = this->vramPointer[frameBufferPixelId + 1]; //TODO: verify order of these two
		returnValue = ((uint32_t)pixel1) << 16 | pixel0;

		IncrementVramAccessHelpers();

		this->remainingWords--;
		if (remainingWords == 0)
		{
			this->inCommand = false;
			this->commandState = 0;
		}		
	}

	return returnValue;
}

void Gpu::SendGP1Command(uint32_t data)
{
	LOG_INFO << "GP1: %08x " << data;
}

uint32_t Gpu::GetGPUStatus() const
{
	return 0x1c000000;
}

uint16_t * Gpu::getVram()
{
	return this->vramPointer;
}
