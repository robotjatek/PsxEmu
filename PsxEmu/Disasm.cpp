#include "Disasm.h"
#include "Memory.h"
#include "R3000A.h"
#include <sstream>


Disasm::Disasm(Memory* pMemory)
{
	this->pMemory = pMemory;
	Enabled = false;
}


Disasm::~Disasm()
{
}

std::string Disasm::DecodeInstruction(uint32_t Instruction)
{
	uint8_t Opcode = (Instruction & 0xFC000000) >> 26;
	std::string ReturnValue = (this->*OpcodePtr[Opcode])(Instruction);
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

std::string Disasm::RtypeFields(uint32_t Instruction)
{
	R3000A* Cpu = pMemory->GetCpu();
	RTypeInstruction ri = Cpu->getRTypeFields(Instruction);
	std::stringstream retval;
	retval
		<< RegisterNames[ri.rd] << " (" << std::hex << Cpu->read_register(ri.rd) << "), "
		<< RegisterNames[ri.rs] << " (" << std::hex << Cpu->read_register(ri.rs) << "), "
		<< RegisterNames[ri.rt] << " (" << std::hex << Cpu->read_register(ri.rt) << ")";

	return retval.str();
}

std::string Disasm::ItypeFields(uint32_t Instruction)
{
	R3000A* Cpu = pMemory->GetCpu();
	ITypeInstruction ii = Cpu->getITypeFields(Instruction);
	std::stringstream retval;
	retval
		<< RegisterNames[ii.rt] << " (" << std::hex << Cpu->read_register(ii.rt) << "), "
		<< RegisterNames[ii.rs] << " (" << std::hex << Cpu->read_register(ii.rs) << "), "
		<< std::hex << ii.immediate;

	return retval.str();
}

std::string Disasm::BEQBNEFields(uint32_t Instruction)
{
	R3000A* Cpu = pMemory->GetCpu();
	ITypeInstruction ii = Cpu->getITypeFields(Instruction);
	std::stringstream retval;
	retval
		<< RegisterNames[ii.rs] << " (" << std::hex << Cpu->read_register(ii.rs) << "), "
		<< RegisterNames[ii.rt] << " (" << std::hex << Cpu->read_register(ii.rt) << "), "
		<< std::hex << ii.immediate;

	return retval.str();
}

std::string Disasm::BranchFields(uint32_t Instruction)
{
	R3000A* Cpu = pMemory->GetCpu();
	ITypeInstruction ii = Cpu->getITypeFields(Instruction);
	std::stringstream retval;
	retval
		<< RegisterNames[ii.rs] << " (" << std::hex << Cpu->read_register(ii.rs) << "), "
		<< std::hex << ii.immediate;

	return retval.str();
}

std::string Disasm::JumpField(uint32_t Instruction)
{
	R3000A* Cpu = pMemory->GetCpu();
	JTypeInstruction ji = Cpu->getJTypeFields(Instruction);
	std::stringstream retval;
	retval
		<< "["<< std::hex<<((Cpu->pc & 0xf0000000) | (ji.target << 2))<< "]";

	return retval.str();
}

std::string Disasm::JALRFields(uint32_t Instruction)
{
	R3000A* Cpu = pMemory->GetCpu();
	RTypeInstruction ri = Cpu->getRTypeFields(Instruction);
	std::stringstream retval;
	retval
		<< RegisterNames[ri.rd] << " (" << std::hex << Cpu->read_register(ri.rd) << "), "
		<< RegisterNames[ri.rs] << " (" << std::hex << Cpu->read_register(ri.rs) << "), ";

	return retval.str();
}

std::string Disasm::LoadStoreFields(uint32_t Instruction)
{
	R3000A* Cpu = pMemory->GetCpu();
	ITypeInstruction ii = Cpu->getITypeFields(Instruction);
	std::stringstream retval;
	retval
		<< RegisterNames[ii.rt] << " (" << std::hex << Cpu->read_register(ii.rt) << "), "
		<< std::hex << ii.immediate << ","
		<< RegisterNames[ii.rs] << " (" << std::hex << Cpu->read_register(ii.rs) << "), "
		<< "[" << ((int16_t)ii.immediate) + Cpu->read_register(ii.rs) << "]";


	return retval.str();
}

std::string Disasm::ShiftFields(uint32_t Instruction)
{
	R3000A* Cpu = pMemory->GetCpu();
	RTypeInstruction ri = Cpu->getRTypeFields(Instruction);
	std::stringstream retval;
	retval
		<< RegisterNames[ri.rd] << " (" << std::hex << Cpu->read_register(ri.rd) << "), "
		<< RegisterNames[ri.rt] << " (" << std::hex << Cpu->read_register(ri.rt) << "), "
		<< (uint32_t)ri.sht;

	return retval.str();
}

std::string Disasm::LUIFields(uint32_t Instruction)
{
	R3000A* Cpu = pMemory->GetCpu();
	ITypeInstruction ii = Cpu->getITypeFields(Instruction);
	std::stringstream retval;
	retval
		<< RegisterNames[ii.rt] << " (" << std::hex << Cpu->read_register(ii.rt) << "), "
		<< std::hex << ii.immediate;

	return retval.str();
}

std::string Disasm::DecodeRtype(uint32_t Instruction)
{
	uint8_t Funct = Instruction & 0x3f;
	return (this->*RtypePtr[Funct])(Instruction);
}

std::string Disasm::DecodeRegImm(uint32_t Instruction)
{
	uint8_t rt = (uint8_t)((Instruction & 0x1F0000) >> 16);
	return (this->*RegImmPtr[rt])(Instruction);
}

std::string Disasm::DecodeCop(uint32_t Instruction)
{
	uint32_t Selector = (Instruction & 0x3E00000) >> 21;
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

std::string Disasm::ADD(uint32_t Instruction) { return "ADD " + RtypeFields(Instruction); };
std::string Disasm::ADDI(uint32_t Instruction) { return "ADDI " + ItypeFields(Instruction); };
std::string Disasm::ADDIU(uint32_t Instruction) { return "ADDIU " + ItypeFields(Instruction); };
std::string Disasm::ADDU(uint32_t Instruction) { return "ADDU " + RtypeFields(Instruction); };
std::string Disasm::AND(uint32_t Instruction) { return "AND " + RtypeFields(Instruction); };
std::string Disasm::ANDI(uint32_t Instruction) { return "ANDI " + ItypeFields(Instruction); };
std::string Disasm::BEQ(uint32_t Instruction) { return "BEQ " + BEQBNEFields(Instruction); };
std::string Disasm::BGEZ(uint32_t Instruction) { return "BGEZ " + BranchFields(Instruction); };
std::string Disasm::BGEZAL(uint32_t Instruction) { return "BGEZAL " + BranchFields(Instruction); };
std::string Disasm::BGTZ(uint32_t Instruction) { return "BGTZ " + BranchFields(Instruction); };
std::string Disasm::BLEZ(uint32_t Instruction) { return "BLEZ " + BranchFields(Instruction); };
std::string Disasm::BLTZ(uint32_t Instruction) { return "BLZT " + BranchFields(Instruction); };
std::string Disasm::BLTZAL(uint32_t Instruction) { return "BLZTAL " + BranchFields(Instruction); };
std::string Disasm::BNE(uint32_t Instruction) { return "BNE " + BEQBNEFields(Instruction); };
std::string Disasm::BREAK(uint32_t) { return "BREAK"; };
std::string Disasm::CFCz(uint32_t Instruction) { return "CFCz"; };
std::string Disasm::COPz(uint32_t Instruction) { return "COPz"; };
std::string Disasm::CTCz(uint32_t Instruction) { return "CTCz"; };
std::string Disasm::DIV(uint32_t Instruction) { return "DIV"; };
std::string Disasm::DIVU(uint32_t Instruction) { return "DIVU"; };
std::string Disasm::J(uint32_t Instruction) { return "J " + JumpField(Instruction); };
std::string Disasm::JAL(uint32_t Instruction) { return "JAL " + JumpField(Instruction); };
std::string Disasm::JALR(uint32_t Instruction) { return "JALR " + JALRFields(Instruction); };
std::string Disasm::JR(uint32_t Instruction) { return "JR " + JALRFields(Instruction); };
std::string Disasm::LB(uint32_t Instruction) { return "LB " + LoadStoreFields(Instruction); };
std::string Disasm::LBU(uint32_t Instruction) { return "LBU " + LoadStoreFields(Instruction); };
std::string Disasm::LH(uint32_t Instruction) { return "LH " + LoadStoreFields(Instruction); };
std::string Disasm::LHU(uint32_t Instruction) { return "LHU " + LoadStoreFields(Instruction); };
std::string Disasm::LUI(uint32_t Instruction) { return "LUI " + LUIFields(Instruction); };
std::string Disasm::LW(uint32_t Instruction) { return "LW " + LoadStoreFields(Instruction); };
std::string Disasm::LWCz(uint32_t Instruction) { return "LWCz " + LoadStoreFields(Instruction); };
std::string Disasm::LWL(uint32_t Instruction) { return "LWL " + LoadStoreFields(Instruction); };
std::string Disasm::LWR(uint32_t Instruction) { return "LWR " + LoadStoreFields(Instruction); };
std::string Disasm::MFCz(uint32_t Instruction) { return "MFCz"; };
std::string Disasm::MFHI(uint32_t Instruction) { return "MFHI"; };
std::string Disasm::MFLO(uint32_t Instruction) { return "MFLO"; };
std::string Disasm::MTCz(uint32_t Instruction) { return "MTCz"; };
std::string Disasm::MTHI(uint32_t Instruction) { return "MTHI"; };
std::string Disasm::MTLO(uint32_t Instruction) { return "MTLO"; };
std::string Disasm::MULT(uint32_t Instruction) { return "MULT"; };
std::string Disasm::MULTU(uint32_t Instruction) { return "MULTU"; };
std::string Disasm::NOR(uint32_t Instruction) { return "NOR " + RtypeFields(Instruction); };
std::string Disasm::OR(uint32_t Instruction) { return "OR " + RtypeFields(Instruction); };
std::string Disasm::ORI(uint32_t Instruction) { return "ORI " + ItypeFields(Instruction); };
std::string Disasm::RFE(uint32_t) { return "RFE"; };
std::string Disasm::SB(uint32_t Instruction) { return "SB " + LoadStoreFields(Instruction); };
std::string Disasm::SH(uint32_t Instruction) { return "SH " + LoadStoreFields(Instruction); };
std::string Disasm::SLL(uint32_t Instruction) { return "SLL " + ShiftFields(Instruction); };
std::string Disasm::SLLV(uint32_t Instruction) { return "SLLV " + RtypeFields(Instruction); };
std::string Disasm::SLT(uint32_t Instruction) { return "SLT " + RtypeFields(Instruction); };
std::string Disasm::SLTI(uint32_t Instruction) { return "SLTI " + ItypeFields(Instruction); };
std::string Disasm::SLTIU(uint32_t Instruction) { return "SLTIU " + ItypeFields(Instruction); };
std::string Disasm::SLTU(uint32_t Instruction) { return "SLTU " + RtypeFields(Instruction); };
std::string Disasm::SRA(uint32_t Instruction) { return "SRA" + ShiftFields(Instruction); };
std::string Disasm::SRAV(uint32_t Instruction) { return "SRAV " + RtypeFields(Instruction); };
std::string Disasm::SRL(uint32_t Instruction) { return "SRL " + ShiftFields(Instruction); };
std::string Disasm::SRLV(uint32_t Instruction) { return "SRLV " + RtypeFields(Instruction); };
std::string Disasm::SUB(uint32_t Instruction) { return "SUB " + RtypeFields(Instruction); };
std::string Disasm::SUBU(uint32_t Instruction) { return "SUBU " + RtypeFields(Instruction); };
std::string Disasm::SW(uint32_t Instruction) { return "SW " + LoadStoreFields(Instruction); };
std::string Disasm::SWCz(uint32_t Instruction) { return "SWCz " + LoadStoreFields(Instruction); };
std::string Disasm::SWL(uint32_t Instruction) { return "SWL " + LoadStoreFields(Instruction); };
std::string Disasm::SWR(uint32_t Instruction) { return "SWR " + LoadStoreFields(Instruction); };
std::string Disasm::SYSCALL(uint32_t Instruction) { return "SYSCALL"; };
std::string Disasm::XOR(uint32_t Instruction) { return "XOR " + RtypeFields(Instruction); };
std::string Disasm::XORI(uint32_t Instruction) { return "XORI " + ItypeFields(Instruction); };
