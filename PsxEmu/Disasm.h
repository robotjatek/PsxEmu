#pragma once
#include <cstdint>

class Disasm
{
public:
	Disasm();
	virtual ~Disasm();
	const char* DecodeInstruction(uint32_t Instruction);
	void Enable();
	void Disable();
	bool IsEnabled();

private:
	bool Enabled;

	typedef const char* (Disasm::*OpcodeTable)(uint32_t Instruction);
	typedef const char* (Disasm::*RtypeTable)(uint32_t Instruction);
	typedef const char* (Disasm::*RegImmTable)(uint32_t Instruction);
	
	const char* DecodeRtype(uint32_t Instruction);
	const char* DecodeRegImm(uint32_t Instruction);
	const char* DecodeCop(uint32_t Instruction);
	const char* CpuNull(uint32_t Instruction) { return "Unknown opcode: " + Instruction; };

	const char* ADD(uint32_t Instruction) { return "ADD"; };
	const char* ADDI(uint32_t Instruction) { return "ADDI"; };
	const char* ADDIU(uint32_t Instruction) { return "ADDIU"; };
	const char* ADDU(uint32_t Instruction) { return "ADDU"; };
	const char* AND(uint32_t Instruction) { return "AND"; };
	const char* ANDI(uint32_t Instruction) { return "ANDI"; };
	const char* BEQ(uint32_t Instruction) { return "BEQ"; };
	const char* BGEZ(uint32_t Instruction) { return "BGEZ"; };
	const char* BGEZAL(uint32_t Instruction) { return "BGEZAL"; };
	const char* BGTZ(uint32_t Instruction) { return "BGTZ"; };
	const char* BLEZ(uint32_t Instruction) { return "BLEZ"; };
	const char* BLTZ(uint32_t Instruction) { return "BLZT"; };
	const char* BLTZAL(uint32_t Instruction) { return "BLZTAL"; };
	const char* BNE(uint32_t Instruction) { return "BNE"; };
	const char* BREAK(uint32_t Instruction) { return "BREAK"; };
	const char* CFCz(uint32_t Instruction) { return "CFCz"; };
	const char* COPz(uint32_t Instruction) { return "COPz"; };
	const char* CTCz(uint32_t Instruction) { return "CTCz"; };
	const char* DIV(uint32_t Instruction) { return "DIV"; };
	const char* DIVU(uint32_t Instruction) { return "DIVU"; };
	const char* J(uint32_t Instruction) { return "J"; };
	const char* JAL(uint32_t Instruction) { return "JAL"; };
	const char* JALR(uint32_t Instruction) { return "JALR"; };
	const char* JR(uint32_t Instruction) { return "JR"; };
	const char* LB(uint32_t Instruction) { return "LB"; };
	const char* LBU(uint32_t Instruction) { return "LBU"; };
	const char* LH(uint32_t Instruction) { return "LH"; };
	const char* LHU(uint32_t Instruction) { return "LHU"; };
	const char* LUI(uint32_t Instruction) { return "LUI"; };
	const char* LW(uint32_t Instruction) { return "LW"; };
	const char* LWCz(uint32_t Instruction) { return "LWCz"; };
	const char* LWL(uint32_t Instruction) { return "LWL"; };
	const char* LWR(uint32_t Instruction) { return "LWR"; };
	const char* MFCz(uint32_t Instruction) { return "MFCz"; };
	const char* MFHI(uint32_t Instruction) { return "MFHI"; };
	const char* MFLO(uint32_t Instruction) { return "MFLO"; };
	const char* MTCz(uint32_t Instruction) { return "MTCz"; };
	const char* MTHI(uint32_t Instruction) { return "MTHI"; };
	const char* MTLO(uint32_t Instruction) { return "MTLO"; };
	const char* MULT(uint32_t Instruction) { return "MULT"; };
	const char* MULTU(uint32_t Instruction) { return "MULTU"; };
	const char* NOR(uint32_t Instruction) { return "NOR"; };
	const char* OR(uint32_t Instruction) { return "OR"; };
	const char* ORI(uint32_t Instruction) { return "ORI"; };
	const char* RFE(uint32_t Instruction) { return "RFE"; };
	const char* SB(uint32_t Instruction) { return "SB"; };
	const char* SH(uint32_t Instruction) { return "SH"; };
	const char* SLL(uint32_t Instruction) { return "SLL"; };
	const char* SLLV(uint32_t Instruction) { return "SLLV"; };
	const char* SLT(uint32_t Instruction) { return "SLT"; };
	const char* SLTI(uint32_t Instruction) { return "SLTI"; };
	const char* SLTIU(uint32_t Instruction) { return "SLTIU"; };
	const char* SLTU(uint32_t Instruction) { return "SLTU"; };
	const char* SRA(uint32_t Instruction) { return "SRA"; };
	const char* SRAV(uint32_t Instruction) { return "SRAV"; };
	const char* SRL(uint32_t Instruction) { return "SRL"; };
	const char* SRLV(uint32_t Instruction) { return "SRLV"; };
	const char* SUB(uint32_t Instruction) { return "SUB"; };
	const char* SUBU(uint32_t Instruction) { return "SUBU"; };
	const char* SW(uint32_t Instruction) { return "SW"; };
	const char* SWCz(uint32_t Instruction) { return "SWCz"; };
	const char* SWL(uint32_t Instruction) { return "SWL"; };
	const char* SWR(uint32_t Instruction) { return "SWR"; };
	const char* SYSCALL(uint32_t Instruction) { return "SYSCALL"; };
	const char* XOR(uint32_t Instruction) { return "XOR"; };
	const char* XORI(uint32_t Instruction) { return "XORI"; };
	const char* ToBeImplemented(uint32_t Instruction) { return "ASD"; };


	RegImmTable RegImmPtr[18] = {
		&Disasm::BLTZ, &Disasm::BGEZ, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		&Disasm::BLTZAL, &Disasm::BGEZAL
	};

	RtypeTable RtypePtr[64] = {
		&Disasm::SLL, &Disasm::ToBeImplemented, &Disasm::SRL, &Disasm::SRA, &Disasm::SLLV, nullptr, &Disasm::SRLV, &Disasm::SRAV,
		&Disasm::JR, &Disasm::JALR, nullptr, nullptr, &Disasm::SYSCALL, &Disasm::BREAK, nullptr, nullptr,
		&Disasm::MFHI, &Disasm::MTHI, &Disasm::MFLO, &Disasm::MTLO, nullptr, nullptr, nullptr, nullptr,
		&Disasm::MULT, &Disasm::MULTU, &Disasm::DIV, &Disasm::DIVU, nullptr, nullptr, nullptr, nullptr,
		&Disasm::ADD, &Disasm::ADDU, &Disasm::SUB, &Disasm::SUBU, &Disasm::AND, &Disasm::OR, &Disasm::XOR, &Disasm::NOR,
		nullptr, nullptr, &Disasm::SLT, &Disasm::SLTU, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
	};

	OpcodeTable OpcodePtr[64] = {
		&Disasm::DecodeRtype, &Disasm::DecodeRegImm, &Disasm::J, &Disasm::JAL, &Disasm::BEQ, &Disasm::BNE, &Disasm::BLEZ, &Disasm::BGTZ,
		&Disasm::ADDI, &Disasm::ADDIU, &Disasm::SLTI, &Disasm::SLTIU, &Disasm::ANDI, &Disasm::ORI, &Disasm::XORI, &Disasm::LUI,
		&Disasm::DecodeCop, &Disasm::DecodeCop, &Disasm::DecodeCop, &Disasm::DecodeCop, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		&Disasm::LB, &Disasm::LH, &Disasm::LWL, &Disasm::LW, &Disasm::LBU, &Disasm::LHU, &Disasm::LWR, nullptr,
		&Disasm::SB, &Disasm::SH, &Disasm::SWL, &Disasm::SW, nullptr, nullptr, &Disasm::SWR, nullptr,
		&Disasm::LWCz, &Disasm::LWCz, &Disasm::LWCz, &Disasm::LWCz, nullptr, nullptr, nullptr, nullptr,
		nullptr, &Disasm::SWCz, &Disasm::SWCz, &Disasm::SWCz, nullptr, nullptr, nullptr, nullptr
	};
};

