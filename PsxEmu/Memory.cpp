#include "Memory.h"
#include <fstream>
#include <iostream>

/* Fact: the PSX has a 4 kB I-cache. If the instruction is in the I-cache, it takes just 1 clock cycle to execute.
The cache hit ratio is 95%, so most code run at full speed.*/


inline uint32_t Memory::transform_virtual_address_to_physical(uint32_t vaddr)
{
	return vaddr & (~0xE0000000);
}

void Memory::set_memory_pointers(uint32_t vaddr)
{
	if (vaddr >= BIOS_START && vaddr <= BIOS_END)
	{
		m_byte_ptr = &bios_area[vaddr - BIOS_START];
	}
	else if (vaddr >= HARDWARE_REGISTERS_START && vaddr <= HARDWARE_REGISTERS_END)
	{
		m_byte_ptr = &m_hardware_registers[vaddr - HARDWARE_REGISTERS_START];
	}
	else if (vaddr >= SCRATCH_PAD_START && vaddr <= SCRATCH_PAD_END)
	{
		m_byte_ptr = &m_scratch_pad[vaddr - SCRATCH_PAD_START];
	}
	else if (vaddr >= PARALLEL_PORT_START && vaddr <= PARALLEL_PORT_END)
	{
		m_byte_ptr = &m_parallel_port[vaddr - PARALLEL_PORT_START];
	}
	else if ((vaddr >= MAIN_MEMORY_START && vaddr <= MAIN_MEMORY_END) || (vaddr >= CACHED_MIRROR_START && vaddr <= CACHED_MIRROR_END) || (vaddr >= UNCACHED_MIRROR_START && vaddr <= UNCACHED_MIRROR_END))
	{
		m_byte_ptr = &m_rawData[transform_virtual_address_to_physical(vaddr)];
	}
	else
	{
		m_byte_ptr = nullptr;
	}

	m_halfword_ptr = reinterpret_cast<uint16_t*>(m_byte_ptr);
	m_word_ptr = reinterpret_cast<uint32_t*>(m_byte_ptr);
}

Memory::Memory()
{
	m_rawData = new uint8_t[MEMORY_SIZE]; //2Mb of RAM in PSX
	memset(m_rawData, 0, MEMORY_SIZE);
	m_parallel_port = new uint8_t[PARALLEL_PORT_SIZE];
	m_scratch_pad = new uint8_t[SCRATCH_PAD_SIZE];
	m_hardware_registers = new uint8_t[HARDWARE_REGISTERS_SIZE];
	bios_area = new uint8_t[BIOS_SIZE];
	memset(&bios_area[0], 0, sizeof(bios_area));
}


Memory::~Memory()
{
	delete[] m_rawData;
	delete[] m_parallel_port;
	delete[] m_scratch_pad;
	delete[] m_hardware_registers;
	delete[] bios_area;
}

//"The RAM is arranged so that the addresses at 0x00xxxxxx, 0xA0xxxxxx, 0x80xxxxxx all point to the same physical memory."


//TODO: status regiszterben a BEV (boot exception vector) 1 és 0 eseteknek utánanézni/tesztelni
uint8_t Memory::read(uint32_t address)
{
	//----------handle mirroring: old way-------------------
	/*if (address >= 0x80000000 && address <= 0x801fffff)
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
	}*/
	//----------handle mirroring: old way-------------------

	set_memory_pointers(address);
	if (m_byte_ptr)
	{
		return *m_byte_ptr;
	}
	else
	{
		return 0; //holes in memory map 
	}
}

uint16_t Memory::read_halfword(uint32_t address)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	set_memory_pointers(address);
	if (m_halfword_ptr)
	{
		return *m_halfword_ptr;
	}
	else
	{
		return 0;
	}
#else
	return read(address) << 8 | read(address + 1); //for systems with big endian compiler
#endif
}

uint32_t Memory::read_word(uint32_t address)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	set_memory_pointers(address);
	if (m_word_ptr)
	{
		return *m_word_ptr;
	}
	else
	{
		return 0;
	}
#else
	return read(address) << 24 | read(address + 1) << 16 | read(address + 2) << 8 | read(address + 3); //for systems with big endian compiler
#endif
}

void Memory::write(uint32_t address, uint8_t data)
{
	/*
	//handle mirroring----------------------------------
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
	//----------handle mirroring------------------
	*/

	set_memory_pointers(address);
	if (m_byte_ptr && address <= BIOS_START && address >= BIOS_END)
	{
		*m_byte_ptr = data;
	}
}

void Memory::write_halfword(uint32_t address, uint16_t data)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	set_memory_pointers(address);
	if (m_halfword_ptr && address <= BIOS_START && address >= BIOS_END)
	{
		*m_halfword_ptr = data;
	}
#else
	uint8_t v1 = data >> 8;
	uint8_t v2 = data & 0xff;
	write(address, v1);
	write(address + 2, v2);
#endif
}

void Memory::write_word(uint32_t address, uint32_t data)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	set_memory_pointers(address);
	if (m_word_ptr && address <= BIOS_START && address >= BIOS_END)
	{
		*m_word_ptr = data;
	}
#else
	uint8_t v1 = data >> 24;
	uint8_t v2 = (data >> 16) & 0xff;
	uint8_t v3 = (data >> 8) & 0xff;
	uint8_t v4 = data & 0xff;
	write(address, v1);
	write(address + 1, v2);
	write(address + 2, v3);
	write(address + 3, v4);
#endif
}

void Memory::load_binary_to_bios_area(std::string filename)
{
	std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		std::streamoff size = file.tellg();
		file.seekg(0, file.beg);
		file.read((char*)&(this->bios_area[0]), size);
		file.close();
	}
}
