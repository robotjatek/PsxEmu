#pragma once
#include <cstdint>
#include <string>

#define BIOS_START 0xbfc00000
#define BIOS_END 0xbfc7ffff
#define BIOS_SIZE 0x80000 //512k
#define MAIN_MEMORY_START 0
#define MAIN_MEMORY_END 0x001fffff
#define CACHED_MIRROR_START 0x80000000
#define CACHED_MIRROR_END 0x801fffff
#define UNCACHED_MIRROR_START 0xa0000000
#define UNCACHED_MIRROR_END 0xa01fffff
#define MEMORY_SIZE 0x1FFFFF //2MB
#define PARALLEL_PORT_SIZE 0x10000 //64k
#define PARALLEL_PORT_START 0x1f000000
#define PARALLEL_PORT_END 0x1f00ffff
#define SCRATCH_PAD_SIZE 0x400 //1k
#define SCRATCH_PAD_START 0x1f800000
#define SCRATCH_PAD_END 0x1f8003ff
#define HARDWARE_REGISTERS_SIZE 0x2000 //8k
#define HARDWARE_REGISTERS_START 0x1f801000
#define HARDWARE_REGISTERS_END 0x1f802fff

class Memory
{
private:
	uint8_t* m_rawData; //2M
	uint8_t* m_parallel_port; //64k
	uint8_t* m_scratch_pad; //1k //???: D-cache?
	uint8_t* m_hardware_registers; //8k
	uint8_t* bios_area; //512k
	inline uint32_t transform_virtual_address_to_physical(uint32_t vaddr);

	uint8_t* m_byte_ptr;
	uint16_t* m_halfword_ptr;
	uint32_t* m_word_ptr;
	inline void set_memory_pointers(uint32_t vaddr);
public:
	Memory();
	~Memory();
	uint8_t read(uint32_t address);
	uint16_t read_halfword(uint32_t address);
	uint32_t read_word(uint32_t address);
	void write(uint32_t address, uint8_t data);
	void write_halfword(uint32_t address, uint16_t data);
	void write_word(uint32_t address, uint32_t data);
	void load_binary_to_bios_area(std::string filename);
};

