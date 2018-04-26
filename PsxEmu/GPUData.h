#pragma once
#include <cstdint>

struct PSXVertex
{
	uint16_t x, y;
	uint8_t r, g, b;
	uint8_t u, v;
};

struct PolygonData
{
	uint16_t ColorLookupTableIndex;
	uint16_t TexturePageIndex;
	PSXVertex vertices[4];
};