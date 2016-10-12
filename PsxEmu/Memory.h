#pragma once
#include <cstdint>
#include <string>

#define BIOS_START 0xbfc00000
#define BIOS_END 0xbfc7ffff
#define MEMORY_SIZE 2097152 //2MB

class Memory
{
private:
	uint8_t* m_rawData;
	uint8_t bios_area[524288]; //512k
public:
	Memory();
	~Memory();
	uint8_t read(uint32_t address);
	void write(uint32_t address, uint8_t data);
	void load_binary_to_bios_area(std::string filename);
};

