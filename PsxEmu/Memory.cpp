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
	if ((vaddr >= BIOS_START_UNCACHED && vaddr <= BIOS_END_UNCACHED))
	{
		m_byte_ptr = &(bios_area[vaddr - BIOS_START_UNCACHED]);
	}
	else if ((vaddr >= UNCACHED_MIRROR_START && vaddr <= UNCACHED_MIRROR_END) || (vaddr >= CACHED_MIRROR_START && vaddr <= CACHED_MIRROR_END) || (vaddr >= MAIN_MEMORY_START && vaddr <= MAIN_MEMORY_END))
	{
		vaddr &= 0x1FFFFFFF;
		m_byte_ptr = &m_rawData[vaddr - MAIN_MEMORY_START];
	}
	else if ((vaddr >= HARDWARE_REGISTERS_START && vaddr <= HARDWARE_REGISTERS_END))
	{
		std::cout << "Hardware registers: " << std::hex << vaddr << std::endl;
		m_byte_ptr = &(m_io_ports[vaddr - HARDWARE_REGISTERS_START]);
		switch (vaddr)
		{
		case 0x1f801060:
			std::cout << "RAM size accessed" << std::endl;
			break;
		case 0x1f801010:
			std::cout << "memory control accessed\n";
			break;
		case 0x1f801000:
			std::cout << "Expansion 1 base address reg\n";
			break;
		case 0x1F801004:
			std::cout << "Expansion 2 base address reg\n";
			break;
		case 0x1f801074:
			std::cout << "Interrupt mask reg\n";
			break;
		}

	}
	else if ((vaddr >= EXPANSION_REGION1_KUSEG_START && vaddr <= EXPANSION_REGION1_KUSEG_END))
	{
		std::cout << "Expansion region 1" << std::hex << vaddr << std::endl;
		m_byte_ptr = &(m_expansion_area1[vaddr - EXPANSION_REGION1_KUSEG_START]);
	}
	else if (vaddr == 0xfffe0130)
	{
		std::cout << "Cache control register accessed. Implement cache!\n";
		m_byte_ptr = &seg2[0];
		//TODO: implement cache
	}
	else
	{
		std::cout << "err\n";
		m_byte_ptr = nullptr;
	}
}

Memory::Memory()
{
	m_rawData = new uint8_t[MEMORY_SIZE]; //2Mb of RAM in PSX
	memset(m_rawData, 0, MEMORY_SIZE);
	m_parallel_port = new uint8_t[PARALLEL_PORT_SIZE];
	m_scratch_pad = new uint8_t[SCRATCH_PAD_SIZE];
	m_io_ports = new uint8_t[HARDWARE_REGISTERS_SIZE];
	memset(&m_io_ports[0], 0, HARDWARE_REGISTERS_SIZE);
	set_memory_pointers(0x1f801000);
	m_word_ptr = reinterpret_cast<uint32_t*>(m_byte_ptr);
	m_word_ptr[0] = 0x1f000000;
	m_word_ptr[1] = 0x1f802000;
	bios_area = new uint8_t[BIOS_SIZE];
	memset(&bios_area[0], 0, sizeof(bios_area));
	m_expansion_area1 = new uint8_t[EXPANSION_REGION1_SIZE];
}


Memory::~Memory()
{
	delete[] m_rawData;
	delete[] m_parallel_port;
	delete[] m_scratch_pad;
	delete[] m_io_ports;
	delete[] bios_area;
	delete[] m_expansion_area1;
}

//"The RAM is arranged so that the addresses at 0x00xxxxxx, 0xA0xxxxxx, 0x80xxxxxx all point to the same physical memory."


//TODO: status regiszterben a BEV (boot exception vector) 1 és 0 eseteknek utánanézni/tesztelni
uint8_t Memory::read(uint32_t address)
{
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
	m_halfword_ptr = reinterpret_cast<uint16_t*>(m_byte_ptr);
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
	m_word_ptr = reinterpret_cast<uint32_t*>(m_byte_ptr);
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
	set_memory_pointers(address);
	if (m_byte_ptr)
	{
		*m_byte_ptr = data;
	}
}

void Memory::write_halfword(uint32_t address, uint16_t data)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	set_memory_pointers(address);
	m_halfword_ptr = reinterpret_cast<uint16_t*>(m_byte_ptr);
	if (m_halfword_ptr && ((address <= BIOS_START || address >= BIOS_END) || (address <= BIOS_START_CACHED || address >= BIOS_END_CACHED) || (address <= BIOS_START_UNCACHED || address >= BIOS_END_UNCACHED)))
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
	m_word_ptr = reinterpret_cast<uint32_t*>(m_byte_ptr);
	if (m_word_ptr)
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
