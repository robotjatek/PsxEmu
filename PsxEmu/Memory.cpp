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
		vaddr = transform_virtual_address_to_physical(vaddr);
		m_byte_ptr = &m_rawData[vaddr - MAIN_MEMORY_START];
	}
	else if ((vaddr >= HARDWARE_REGISTERS_START && vaddr <= HARDWARE_REGISTERS_END))
	{
		//std::cout << "Hardware registers: " << std::hex << vaddr << std::endl;
		m_byte_ptr = &(m_io_ports[vaddr - HARDWARE_REGISTERS_START]);

		if (vaddr >= 0x1F801C00 && vaddr <= 0x1F801FFF)
		{
			//	printf("SPU register accessed: %08x\n", vaddr);
				//TODO: implement SPU
		}
		else if (vaddr >= 0x1f801000 && vaddr <= 0x1f801020)
		{
			//Memory control 1
			switch (vaddr)
			{
			case 0x1f801000:
				printf("Expansion 1 Base Address\n");
				break;
			case 0x1f801004:
				printf("Expansion 2 Base Address\n");
				break;
			case 0x1f801008:
				printf("Expansion 1 Delay/Size\n");
				break;
			case 0x1f80100c:
				printf("Expansion 3 Delay/Size \n");
				break;
			case 0x1f801010:
				printf("BIOS ROM    Delay/Size\n");
				break;
			case 0x1f801014:
				printf("SPU_DELAY   Delay/Size\n");
				break;
			case 0x1f801018:
				printf("CDROM_DELAY Delay/Size\n");
				break;
			case 0x1f80101c:
				printf("Expansion 2 Delay/Size\n");
				break;
			case 0x1f801020:
				printf("COM_DELAY / COMMON_DELAY\n");
				break;
			default:
				printf("Unhandled access: %08x\n", vaddr);
				break;
			}
		}
		else
		{
			switch (vaddr)
			{
			case 0x1f801060:
				std::cout << "RAM size accessed" << std::endl;
				break;
			case 0x1f801070:
//				std::cout << "Interrupt status reg\n";
				break;
			case 0x1f801074:
//				std::cout << "Interrupt mask reg\n";
				break;
			case 0x1f801810:
				printf("Gpu port\n");
				break;
			case 0x1f801814:
				printf("Gpu port\n");
				break;
			default:
				printf("Unhandled IO port: %08x\n", vaddr);
				break;
			}
		}

	}
	else if ((vaddr >= EXPANSION_REGION1_KUSEG_START && vaddr <= EXPANSION_REGION1_KUSEG_END))
	{
		std::cout << "Expansion region 1: " << std::hex << vaddr << std::endl;
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
		//printf("err addr: %08x\n",vaddr);
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
	gpu = new Gpu;
	dma = new Dma(this);
}


Memory::~Memory()
{
	delete[] m_rawData;
	delete[] m_parallel_port;
	delete[] m_scratch_pad;
	delete[] m_io_ports;
	delete[] bios_area;
	delete[] m_expansion_area1;
	delete dma;
	delete gpu;
}

//"The RAM is arranged so that the addresses at 0x00xxxxxx, 0xA0xxxxxx, 0x80xxxxxx all point to the same physical memory."


uint8_t Memory::read(uint32_t address)
{
	if (address >= 0x1F801080 && address <= 0x1F8010FC)
	{
		//DMA
		switch (address)
		{
		case 0x1f8010f0:
		{
			//DMA Control reg
			return (uint8_t)dma->ReadControl(); //TODO: check 8bit and 16bit reads from hardware registers
			break;
		}
		case 0x1f8010f4:
		{
			// DMA Interrupt Register 
			return (uint8_t)dma->ReadInterrupt();
			break;
		}
		default:
		{
			printf("Unhandled DMA register");
			break;
		}
		}
	}
	else
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

	return 0;
}

uint16_t Memory::read_halfword(uint32_t address)
{
	if (address >= 0x1F801080 && address <= 0x1F8010FC)
	{
		//DMA
		switch (address)
		{
		case 0x1f8010f0:
		{
			//DMA Control reg
			return (uint16_t)dma->ReadControl();
			break;
		}
		case 0x1f8010f4:
		{
			// DMA Interrupt Register 
			return (uint16_t)dma->ReadInterrupt();
			break;
		}
		default:
		{
			printf("Unhandled DMA register");
			break;
		}
		}
	}
	else
	{
		set_memory_pointers(address);
		if (m_halfword_ptr)
		{
			return *m_halfword_ptr;
		}
		else
		{
			return 0;
		}
	}

	return 0;
}

uint32_t Memory::read_word(uint32_t address)
{
	if (address >= 0x1F801080 && address <= 0x1F8010FC)
	{
		//DMA
		return dma->ReadFromDMARegister(address);
	}
	else if (address == 0x1f801810)
	{
		return gpu->GetGPURead();
	}
	else if (address == 0x1f801814)
	{
		return gpu->GetGPUStatus();
	}
	else
	{
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
	}
//	return 0;
}

void Memory::write(uint32_t address, uint8_t data)
{
	if (address >= 0x1F801080 && address <= 0x1F8010FC)
	{
		//DMA
		switch (address)
		{
		case 0x1f8010f0:
		{
			//DMA Control reg
			dma->WriteControl(data);
			break;
		}
		case 0x1f8010f4:
		{
			// DMA Interrupt Register 
			dma->WriteInterrupt(data);
			break;
		}
		default:
		{
			printf("Unhandled DMA register");
			break;
		}
		}
	}
	else
	{
		set_memory_pointers(address);
		if (m_byte_ptr)
		{
			*m_byte_ptr = data;
		}
	}
}

void Memory::write_halfword(uint32_t address, uint16_t data)
{
	if (address >= 0x1F801080 && address <= 0x1F8010FC)
	{
		//DMA
		switch (address)
		{
		case 0x1f8010f0:
		{
			//DMA Control reg
			dma->WriteControl(data);
			break;
		}
		case 0x1f8010f4:
		{
			// DMA Interrupt Register 
			dma->WriteInterrupt(data);
			break;
		}
		default:
		{
			printf("Unhandled DMA register");
			break;
		}
		}
	}
	else
	{
		set_memory_pointers(address);
		m_halfword_ptr = reinterpret_cast<uint16_t*>(m_byte_ptr);
		if (m_halfword_ptr && ((address <= BIOS_START || address >= BIOS_END) || (address <= BIOS_START_CACHED || address >= BIOS_END_CACHED) || (address <= BIOS_START_UNCACHED || address >= BIOS_END_UNCACHED)))
		{
			*m_halfword_ptr = data;
		}
	}
}

void Memory::write_word(uint32_t address, uint32_t data)
{
	if (address >= 0x1F801080 && address <= 0x1F8010FC)
	{
		//DMA
		dma->WriteToDMARegister(address, data);
	}
	else if (address == 0x1f801810)
	{
		gpu->SendGP0Command(data);
	}
	else if (address == 0x1f801814)
	{
		gpu->SendGP1Command(data);
	}
	else
	{
		set_memory_pointers(address);
		m_word_ptr = reinterpret_cast<uint32_t*>(m_byte_ptr);
		if (m_word_ptr)
		{
			*m_word_ptr = data;
		}
	}
}

bool Memory::load_binary_to_bios_area(std::string filename)
{
	std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		std::streamoff size = file.tellg();
		file.seekg(0, file.beg);
		file.read((char*)&(this->bios_area[0]), size);
		file.close();
		return true;
	}
	return false;
}

void Memory::SetIStatFields(uint32_t toSet)
{
	m_io_ports[I_STAT - HARDWARE_REGISTERS_START] |= toSet;
}

void Memory::DisableIStatFields(uint32_t ToDisable)
{
	m_io_ports[I_STAT - HARDWARE_REGISTERS_START] &= ~ToDisable;
}

uint32_t Memory::GetIStatField()
{
	return m_io_ports[I_STAT - HARDWARE_REGISTERS_START];
}

uint32_t Memory::GetIMaskField()
{
	return m_io_ports[I_MASK - HARDWARE_REGISTERS_START];
}
