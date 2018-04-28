#include "Gpu.h"
#include <iostream>


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
		renderer.PushPolygons(currentPolygon, polyCount);
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
		renderer.PushPolygons(currentPolygon, 4);
	}
}

Gpu::Gpu(): vram(new uint8_t[VRAM_SIZE])
{
	this->inCommand = false;
	this->commandState = 0;
	this->remainingWords = 0;
}


Gpu::~Gpu()
{
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

void Gpu::SendGP0Command(uint32_t data)
{
	uint8_t command = (uint8_t)((data & 0xFF000000) >> 24);
	
	if (!inCommand)
	{
		switch (command)
		{
		case 0x00:
			printf("GP0 NOP\n");
			break;
		case 0x01:
			printf("GP0 CLEAR CACHE\n");
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
			printf("Implement GP0(E1h)\n");
			break;
		case 0xe2:
			// TODO:  GP0(E2h) - Texture Window setting
			printf("Implement GP0(E2h)\n");
			break;
		case 0xe3:
			// TODO: GP0(E3h) - Set Drawing Area top left (X1,Y1)
			printf("Implement GP0(E3h)\n");
			break;
		case 0xe4:
			// TODO:  GP0(E4h) - Set Drawing Area bottom right (X2,Y2)
			printf("Implement GP0(E4h)\n");
			break;
		case 0xe5:
		{
			int16_t offsetX = data & 0x7ff;
			int16_t offsetY = (data & 0x3ff800) >> 11;
			renderer.UpdateDrawingOffset(offsetX, offsetY);
			//XXX: based on noca$h specs. "Everything you wanted to ask..." says only bits 13..11 are the Y offset

			renderer.SwapBuffers(); //TODO: remove this hack
			break;
		}
		case 0xe6:
			//TODO: GP0(E6h) - Mask Bit Setting
			printf("Implement GP0(E6h)\n");
			break;
		default:
			printf("NOT IMPLEMENTED GP0 COMMAND!\n");
			printf("GP0: %08x\n", data);
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
			if (commandState == 0)
			{
				uint16_t x, y;
				x = data & 0x0000FFFF;
				y = ((data & 0xFFFF0000) >> 16);
			}
			else if (commandState == 1)
			{
				uint16_t xSize, ySize;
				xSize = data & 0x0000FFFF;
				ySize = ((data & 0xFFFF0000) >> 16);
				uint32_t imagesize = xSize * ySize;
				imagesize = (imagesize + 1) & 0xFFFFFFFE;
				this->remainingWords = imagesize / 2;
			}
			else
			{
				//TODO: elfogadni a memóriába jövõ image datát
				this->remainingWords--;
				if (remainingWords == 0)
				{
					this->inCommand = false;
					this->commandState = 0;
				}
			}

			commandState++;
			break;
		}
		case 0xc0:
		{
			if (commandState == 0)
			{
				uint16_t x, y;
				x = data & 0x0000FFFF;
				y = ((data & 0xFFFF0000) >> 16);
			}
			else if (commandState == 1)
			{
				uint16_t xSize, ySize;
				xSize = data & 0x0000FFFF;
				ySize = ((data & 0xFFFF0000) >> 16);
				uint32_t imagesize = xSize * ySize;
				imagesize = (imagesize + 1) & 0xFFFFFFFE;
				this->remainingWords = imagesize / 2;
			}

			commandState++;
			break;
		}
		default:
			printf("Something went wrong\n");
			break;
		}
	}
}

uint32_t Gpu::GetGPURead()
{
	if (inCommand && currentCommand == 0xc0)
	{
		//TODO: kiolvasni a vramból a kért adatot
		this->remainingWords--;
		if (remainingWords == 0)
		{
			this->inCommand = false;
			this->commandState = 0;
		}		
	}

	return uint32_t();
}

void Gpu::SendGP1Command(uint32_t data)
{
	printf("GP1: %08x\n", data);
}

uint32_t Gpu::GetGPUStatus() const
{
	return 0x1c000000;
}
