#pragma once
#include "Memory.h"
#include "Cop0.h"
#include <cstdint>
#include <iostream>
#include "ICoprocessor.h"

struct Instruction
{
	uint8_t op : 6;
	uint32_t rest : 26;
};

//register type instruction
struct RTypeInstruction
{
	uint8_t op : 6; //operation code
	uint8_t rs : 5; //source register
	uint8_t rt : 5; //target register
	uint8_t rd : 5; //destination register
	uint8_t sht : 5; //shift amount
	uint8_t	funct : 5; //function field
};

//immediate type instruction
struct ITypeInstruction
{
	uint8_t op : 6;
	uint8_t rs : 5;
	uint8_t rt : 5;
	uint16_t immediate : 16;
};

struct JTypeInstruction
{
	uint8_t op : 6;
	uint32_t target : 24;
};


class R3000A
{
public:
	friend class Cop0;

	//Cop0 cop0;
	ICoprocessor* m_copx[4];
	Cop0* m_cop0;
	Memory& m_memory;
	uint32_t registers[32];
	uint32_t hi;
	uint32_t lo;
	uint32_t pc;
	bool kernel_mode;
	bool in_branch;
	bool exception_pending;
	bool delay_slot;
	uint32_t delay_slot_address;

	inline uint32_t Fetch();
	inline void Decode(uint32_t instruction_word);
	bool is_running;
	inline void write_register(uint8_t reg, uint32_t data);
	inline uint32_t read_register(uint8_t reg);
	inline struct RTypeInstruction getRTypeFields(uint32_t opcode);
	inline struct ITypeInstruction getITypeFields(uint32_t opcode);
	inline struct JTypeInstruction getJTypeFields(uint32_t opcode);

	typedef void (R3000A::*RTypeInstructionPtr)(uint8_t rd, uint8_t rs, uint8_t rt);
	RTypeInstructionPtr rtypes[0x3F]; //0b00111111
	typedef void (R3000A::*ITypeInstructionPtr)(uint8_t rt, uint8_t rs, uint16_t imm);
	ITypeInstructionPtr itypes[0x3F];
	typedef void (R3000A::*ITypeInstructionPtr)(uint8_t rt, uint8_t rs, uint16_t imm);
	ITypeInstructionPtr regimm_itypes[0x2];
	typedef void (R3000A::*JTypeInstructionPtr)(uint32_t target);
	JTypeInstructionPtr jtypes[0x3F]; 

	inline void RtypeNull(uint8_t rd, uint8_t rs, uint8_t rt);
	inline void ItypeNull(uint8_t base, uint8_t rt, uint16_t offset);
	inline void JtypeNull(uint32_t target);
	inline void Null();

	inline void CallRegimmFunc(uint8_t rt, uint8_t rs, uint16_t imm);

	inline void Add(uint8_t rd, uint8_t rs, uint8_t rt);
	inline void Addi(uint8_t rt, uint8_t rs, uint16_t imm);
	inline void Addiu(uint8_t rt, uint8_t rs, uint16_t imm);
	inline void Addu(uint8_t rd, uint8_t rs, uint8_t rt);
	inline void And(uint8_t rd, uint8_t rs, uint8_t rt);
	inline void Andi(uint8_t rt, uint8_t rs, uint16_t imm);
	inline void Beq(uint8_t rt, uint8_t rs, uint16_t imm);
	inline void Bgez(uint8_t rt, uint8_t rs, uint16_t offset);
	inline void Bgezal(uint8_t rt, uint8_t rs, uint16_t offset);
	inline void Bgtz(uint8_t rt, uint8_t rs, uint16_t imm);
	inline void Blez(uint8_t rt, uint8_t rs, uint16_t imm);
	inline void Bltz(uint8_t rt, uint8_t rs, uint16_t imm);
	inline void Bltzal(uint8_t rt, uint8_t rs, uint16_t imm);
	inline void Bne(uint8_t rt, uint8_t rs, uint16_t imm);
	inline void Break(uint8_t rd, uint8_t rs, uint8_t rt);
	inline void Copz(uint32_t cop_fun); //implemented in the decode function
	inline void Div(uint8_t rd, uint8_t rs, uint8_t rt);
	inline void Divu(uint8_t rd, uint8_t rs, uint8_t rt);
	inline void J(uint32_t target);
	inline void Jal(uint32_t target);
	inline void Jalr(uint8_t rd, uint8_t rs, uint8_t rt);
	inline void Jr(uint8_t rd, uint8_t rs, uint8_t rt);
	inline void Lb(uint8_t base, uint8_t rt, uint16_t offset);
	inline void Lbu(uint8_t base, uint8_t rt, uint16_t offset);
public:
	R3000A(Memory& mem);
	~R3000A();
	void Run();
	void  Step();

};



