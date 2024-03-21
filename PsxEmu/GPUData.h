#pragma once
#include <cstdint>

struct PSXVertex
{
	uint16_t x, y;
	uint8_t r, g, b;
	uint8_t u, v;
};

struct TexturePage
{
	uint8_t tp : 2; // bit 8-7 (4bit clut/8bit clut/15 bit direct mode)
	uint8_t abr : 2; // bit 6-5
	uint8_t ty : 1; // bit 4 (0:0, 1: 256)
	uint8_t tx : 4;  // bit 3-0 (x*64)
};

struct PolygonData
{
	uint16_t ColorLookupTableIndex;
	/// <summary>
	/// A page is 256*256px
	/// 64 pixels wide for 4bit CLUT, 128 pixels wide for 8bit CLUT and 256 pixels wide for 15-bit direct 
	/// </summary>
	TexturePage page;
	PSXVertex vertices[4];
};