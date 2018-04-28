#pragma once
#include <cstdint>
#include <string>
#include <fstream>
#include "Gpu.h"
#include "DMA.h"
#include "R3000A.h"

#define BIOS_START 0x1fc00000
#define BIOS_END 0x1fc7ffff
#define BIOS_START_UNCACHED 0xbfc00000
#define BIOS_END_UNCACHED 0xbfc7ffff
#define BIOS_START_CACHED 0x9fc00000
#define BIOS_END_CACHED 0x9fc7ffff
#define BIOS_SIZE 0x80000 //512k
#define MAIN_MEMORY_START 0
#define MAIN_MEMORY_END 0x001fffff
#define CACHED_MIRROR_START 0x80000000
#define CACHED_MIRROR_END 0x801fffff
#define UNCACHED_MIRROR_START 0xa0000000
#define UNCACHED_MIRROR_END 0xa01fffff
#define MEMORY_SIZE 0x200000 //2MB
#define PARALLEL_PORT_SIZE 0x10000 //64k
#define PARALLEL_PORT_START 0x1f000000
#define PARALLEL_PORT_END 0x1f00ffff
#define SCRATCH_PAD_SIZE 0x400 //1k
#define SCRATCH_PAD_START 0x1f800000
#define SCRATCH_PAD_END 0x1f8003ff
#define SCRATCH_PAD_KSEG0_START 0x9F800000
#define SCRATCH_PAD_KSEG0_END 0x9F8003FF
#define HARDWARE_REGISTERS_SIZE 0x2000 //8k
#define HARDWARE_REGISTERS_START 0x1f801000
#define HARDWARE_REGISTERS_END 0x1f802fff

#define EXPANSION_REGION1_KUSEG_START 0x1F000000
#define EXPANSION_REGION1_KUSEG_END	0x1F002000
#define EXPANSION_REGION1_SIZE 0x2000

#define I_STAT 0x1F801070
#define I_MASK 0x1F801074

class Memory
{
private:
	uint8_t* const m_rawData; //2M
	uint8_t* const m_parallel_port; //64k
	uint8_t* const m_scratch_pad; //1k //???: D-cache?
	uint8_t* const m_io_ports; //8k
	uint8_t* const bios_area; //512k
	uint8_t seg2[4];
	uint8_t* const m_expansion_area1;
	static uint32_t transform_virtual_address_to_physical(uint32_t vaddr);
	template <class TYPE>
	TYPE* SetMemoryPointer(uint32_t vaddr);
	void DumpMemoryHex(std::fstream& Stream);
	void DumpMemoryASCII(std::fstream& Stream);
	bool InitOK;

	Dma* const dma;
	Gpu* const gpu;
	R3000A* const r3000a;
public:
	Memory();
	~Memory();
	bool load_binary_to_bios_area(std::string filename);

	template <class TYPE>
	TYPE Read(uint32_t address);
	template <class TYPE>
	void Write(uint32_t address, TYPE data);
	
	enum IRQStatFields
	{
		VBLANK = 0x1, //bit 0
		GPU = 0x2, //bit 1
		CDROM = 0x4, //bit 2
		DMA = 0x8, //bit 3
		TMR0 = 0x10, //bit 4
		TMR1 = 0x20, //bit 5
		TMR2 = 0x40, //bit 6
		ControllerAndMemoryCard = 0x80, //bit 7
		SIO = 0x100, //bit 8
		SPU = 0x200, //bit 9
		ControllerLightPen = 0x400, //bit 10 (??? what is lightpen?)
	};
	void SetIStatFields(uint32_t toSet);
	void DisableIStatFields(uint32_t ToDisable);
	uint32_t GetIStatField() const;
	uint32_t GetIMaskField() const;
	const R3000A* GetCpu() const;
	void DumpMemory();
	void RunSystem() const;
	void StopSystem() const;
	void StartLogging();
};

template<class TYPE>
inline TYPE * Memory::SetMemoryPointer(uint32_t vaddr)
{
	uint8_t* ptr = nullptr;
	if ((vaddr >= BIOS_START_UNCACHED && vaddr <= BIOS_END_UNCACHED))
	{
		ptr = &(bios_area[vaddr - BIOS_START_UNCACHED]);
	}
	else if ((vaddr >= UNCACHED_MIRROR_START && vaddr <= UNCACHED_MIRROR_END) || (vaddr >= CACHED_MIRROR_START && vaddr <= CACHED_MIRROR_END) || (vaddr >= MAIN_MEMORY_START && vaddr <= MAIN_MEMORY_END))
	{
		vaddr = transform_virtual_address_to_physical(vaddr);
		ptr = &m_rawData[vaddr - MAIN_MEMORY_START];
	}
	else if ((vaddr >= HARDWARE_REGISTERS_START && vaddr <= HARDWARE_REGISTERS_END))
	{
		//std::cout << "Hardware registers: " << std::hex << vaddr << std::endl;
		ptr = &(m_io_ports[vaddr - HARDWARE_REGISTERS_START]);

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
								//std::cout << "Interrupt status reg\n";
				break;
			case 0x1f801074:
								//std::cout << "Interrupt mask reg\n";
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
		ptr = &(m_expansion_area1[vaddr - EXPANSION_REGION1_KUSEG_START]);
	}
	else if (vaddr == 0xfffe0130)
	{
		std::cout << "Cache control register accessed. Implement cache!\n";
		ptr = &seg2[0];
		//TODO: implement cache
	}
	else
	{
		printf("err addr: %08x\n", vaddr);
		r3000a->StopLogging();
		r3000a->Stop();
		DumpMemory();
		ptr = nullptr;
		//exit(-1);
	}

	if(ptr)
	{
		return reinterpret_cast<TYPE*>(ptr);
	}
	else
	{
		return nullptr; //nullptr cannot be converted with reinterpret cast
	}
}

template<class TYPE>
inline TYPE Memory::Read(uint32_t address)
{
	if (address >= 0x1F801080 && address <= 0x1F8010FC)
	{
		//DMA
		return static_cast<TYPE>(dma->ReadFromDMARegister(address));
	}
	else if (address == 0x1f801810)
	{
		return static_cast<TYPE>(gpu->GetGPURead());
	}
	else if (address == 0x1f801814)
	{
		//r3000a->StopLogging();
		return static_cast<TYPE>(gpu->GetGPUStatus());
	}
	else
	{
		TYPE* ptr = SetMemoryPointer<TYPE>(address);
		if (ptr)
		{
			return static_cast<TYPE>((*ptr));
		}
		else
		{
			return 0;
		}
	}
}

template<class TYPE>
void Memory::Write(uint32_t address, TYPE data)
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
		TYPE* ptr = SetMemoryPointer<TYPE>(address);
		if (ptr)
		{
			*ptr = data;
		}
	}
}