#include "Disasm.h"



Disasm::Disasm()
{
	Enabled = false;
}


Disasm::~Disasm()
{
}

const char* Disasm::DecodeInstruction(uint32_t Instruction)
{
	uint8_t Opcode = (Instruction & 0xFC000000) >> 26;
	const char* ReturnValue = (this->*OpcodePtr[Opcode])(Instruction);
	return ReturnValue;
}

void Disasm::Enable()
{
	Enabled = true;
}

void Disasm::Disable()
{
	Enabled = false;
}

bool Disasm::IsEnabled()
{
	return Enabled;
}

const char * Disasm::DecodeRtype(uint32_t Instruction)
{
	uint8_t Funct = Instruction & 0x3f;
	return (this->*RtypePtr[Funct])(Instruction);
}

const char * Disasm::DecodeRegImm(uint32_t Instruction)
{
	uint8_t rt = (uint8_t)((Instruction & 0x1F0000) >> 16);
	return (this->*RegImmPtr[rt])(Instruction);
}

const char * Disasm::DecodeCop(uint32_t Instruction)
{
	uint8_t Selector = (Instruction & 0x3E00000) >> 21;
	switch (Selector)
	{
	case 0:
		return MFCz(Instruction);
		break;
	case 2:
		return CFCz(Instruction);
		break;
	case 4:
		return MTCz(Instruction);
		break;
	case 6:
		return CTCz(Instruction);
		break;
	case 16:
		if ((Instruction & 0x3f) == 0x10)
		{
			return RFE(Instruction);
		}
		else
		{
			return COPz(Instruction);
		}
		break;
	default:
		return CpuNull(Instruction);
		break;
	}
}
