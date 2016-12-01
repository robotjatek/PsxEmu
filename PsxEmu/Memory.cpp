#include "Memory.h"
#include <iostream>
#include <cstring>

/* Fact: the PSX has a 4 kB I-cache. If the instruction is in the I-cache, it takes just 1 clock cycle to execute.
The cache hit ratio is 95%, so most code run at full speed.*/


uint32_t Memory::transform_virtual_address_to_physical(uint32_t vaddr)
{
	return vaddr & (~0xE0000000);
}

void Memory::DumpMemoryHex(std::fstream& Stream)
{
	for (unsigned int i = 0; i < MAIN_MEMORY_END; i += 8)
	{
		Stream << std::hex << i << " \t\t";
		for (unsigned int j = 0; j < 8; j++)
		{
			Stream << (uint32_t)Read<uint8_t>(i + j) << " ";
		}
		Stream << std::endl;
	}
}

void Memory::DumpMemoryASCII(std::fstream & Stream)
{
	for (unsigned int i = 0; i < MAIN_MEMORY_END; i += 8)
	{
		Stream << std::hex << i << " \t\t";
		for (unsigned int j = 0; j < 8; j++)
		{
			Stream << std::dec << Read<uint8_t>(i + j) << "";
		}
		Stream << std::endl;
	}
}

Memory::Memory() :
m_rawData(new uint8_t[MEMORY_SIZE]), //2Mb of RAM in PSX
m_parallel_port(new uint8_t[PARALLEL_PORT_SIZE]),
m_scratch_pad(new uint8_t[SCRATCH_PAD_SIZE]),
m_io_ports(new uint8_t[HARDWARE_REGISTERS_SIZE]),
bios_area(new uint8_t[BIOS_SIZE]),
m_expansion_area1(new uint8_t[EXPANSION_REGION1_SIZE]),
dma(new Dma(this)),
gpu(new Gpu),
r3000a(new R3000A(this))
{
	memset(m_rawData, 0, MEMORY_SIZE);
	memset(&m_parallel_port[0], 0, PARALLEL_PORT_SIZE);
	memset(&m_scratch_pad[0], 0, SCRATCH_PAD_SIZE);
	memset(&m_io_ports[0], 0, HARDWARE_REGISTERS_SIZE);
	uint32_t* ptr = SetMemoryPointer<uint32_t>(0x1f801000);
	ptr[0] = 0x1f000000;
	ptr[1] = 0x1f802000;
	memset(&bios_area[0], 0, sizeof(bios_area));
	memset(&m_expansion_area1[0], 0, EXPANSION_REGION1_SIZE);
	if (!load_binary_to_bios_area("SCPH1001.BIN"))
	{
		std::cout << "Failed to load BIOS image\n";
	}
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
	delete r3000a;
}

//"The RAM is arranged so that the addresses at 0x00xxxxxx, 0xA0xxxxxx, 0x80xxxxxx all point to the same physical memory."


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

uint32_t Memory::GetIStatField() const
{
	return m_io_ports[I_STAT - HARDWARE_REGISTERS_START];
}

uint32_t Memory::GetIMaskField() const
{
	return m_io_ports[I_MASK - HARDWARE_REGISTERS_START];
}

const R3000A * Memory::GetCpu() const
{
	return r3000a;
}

void Memory::DumpMemory()
{
	std::fstream MemoryDumpStream;
	MemoryDumpStream.open("MemoryDump.txt", std::ios::out);
	DumpMemoryHex(MemoryDumpStream);
	MemoryDumpStream << "---------------------------------------------------------------------------" << std::endl;
	DumpMemoryASCII(MemoryDumpStream);
	MemoryDumpStream.flush();
	MemoryDumpStream.close();
}

void Memory::RunSystem() const
{
	r3000a->Run();
}
