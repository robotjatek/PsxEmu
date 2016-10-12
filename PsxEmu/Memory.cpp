#include "Memory.h"
#include <fstream>

/* Fact: the PSX has a 4 kB I-cache. If the instruction is in the I-cache, it takes just 1 clock cycle to execute. 
The cache hit ratio is 95%, so most code run at full speed.*/


Memory::Memory()
{
	m_rawData = new uint8_t[MEMORY_SIZE]; //2Mb of RAM in PSX
	memset(m_rawData, 0, MEMORY_SIZE);
	memset(&bios_area[0], 0, sizeof(bios_area));
}


Memory::~Memory()
{
	delete[] m_rawData;
}

//"The RAM is arranged so that the addresses at 0x00xxxxxx, 0xA0xxxxxx, 0x80xxxxxx all point to the same physical memory."


//TODO: egyszerûbb lenne a felsõ 3 bitet nullázni egy logikai éssel
//TODO: status regiszterben a BEV (boot exception vector) 1 és 0 eseteknek utánanézni/tesztelni
uint8_t Memory::read(uint32_t address)
{
	//handle mirroring
	if (address >= 0x80000000 && address <= 0x801fffff)
	{
		address -= 0x80000000;
	}
	else if (address >= 0xa0000000 && address <= 0xa01fffff)
	{
		address -= 0xa0000000;
	}
	else if (address >= BIOS_START && address <= BIOS_END)
	{
		return bios_area[address - BIOS_START];
	}
	
	return m_rawData[address];
}

//TODO: scratchpad?

void Memory::write(uint32_t address, uint8_t data)
{
	//handle mirroring
	if (address >= 0x80000000 && address <= 0x801fffff)
	{
		address -= 0x80000000;
	}
	else if (address >= 0xa0000000 && address <= 0xa01fffff)
	{
		address -= 0xa0000000;
	}
	else if (address >= BIOS_START && address <= BIOS_END)
	{
		//ide írni elvileg nem szabadna...
		throw 0;
	}

	m_rawData[address] = data;
}

void Memory::load_binary_to_bios_area(std::string filename)
{
	std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		std::streamoff size = file.tellg();
		file.seekg(0,file.beg);
		file.read((char*)&(this->bios_area[0]), size);
		file.close();
	}
}
