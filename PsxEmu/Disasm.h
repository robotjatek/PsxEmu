#pragma once
#include <cstdint>
#include <string>

class Memory;

class Disasm
{
public:
	Disasm(Memory* pMemory);
	virtual ~Disasm();
	std::string DecodeInstruction(uint32_t Instruction);
	void Enable();
	void Disable();
	bool IsEnabled() const;

private:
	bool Enabled;
	Memory* pMemory;

	typedef std::string (Disasm::*OpcodeTable)(uint32_t Instruction);
	typedef std::string (Disasm::*RtypeTable)(uint32_t Instruction);
	typedef std::string (Disasm::*RegImmTable)(uint32_t Instruction);

	const std::string RegisterNames[32] = {
		"$zero", "$at", "$v0", "$v1", "$a0", "$a1",	"$a2", "$a3",
		"$t0", "$t1", "$t2", "$t3",	"$t4", "$t4", "$t6", "$t7",
		"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
		"$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"
	};
	
	std::string RtypeFields(uint32_t Instruction);
	std::string ItypeFields(uint32_t Instruction);
	std::string BEQBNEFields(uint32_t Instruction);
	std::string BranchFields(uint32_t Instruction);
	std::string JumpField(uint32_t Instruction);
	std::string JALRFields(uint32_t Instruction);
	std::string LoadStoreFields(uint32_t Instruction);
	std::string ShiftFields(uint32_t Instruction);
	std::string LUIFields(uint32_t Instruction);
	std::string DecodeRtype(uint32_t Instruction);
	std::string DecodeRegImm(uint32_t Instruction);
	std::string DecodeCop(uint32_t Instruction);
	std::string CpuNull(uint32_t Instruction) { return "Unknown opcode: " + Instruction; };

	std::string ADD(uint32_t Instruction);
	std::string ADDI(uint32_t Instruction);
	std::string ADDIU(uint32_t Instruction);
	std::string ADDU(uint32_t Instruction);
	std::string AND(uint32_t Instruction);
	std::string ANDI(uint32_t Instruction);
	std::string BEQ(uint32_t Instruction);
	std::string BGEZ(uint32_t Instruction);
	std::string BGEZAL(uint32_t Instruction);
	std::string BGTZ(uint32_t Instruction);
	std::string BLEZ(uint32_t Instruction);
	std::string BLTZ(uint32_t Instruction);
	std::string BLTZAL(uint32_t Instruction);
	std::string BNE(uint32_t Instruction);
	std::string BREAK(uint32_t Instruction);
	std::string CFCz(uint32_t Instruction);
	std::string COPz(uint32_t Instruction);
	std::string CTCz(uint32_t Instruction);
	std::string DIV(uint32_t Instruction);
	std::string DIVU(uint32_t Instruction);
	std::string J(uint32_t Instruction);
	std::string JAL(uint32_t Instruction);
	std::string JALR(uint32_t Instruction);
	std::string JR(uint32_t Instruction);
	std::string LB(uint32_t Instruction);
	std::string LBU(uint32_t Instruction);
	std::string LH(uint32_t Instruction);
	std::string LHU(uint32_t Instruction);
	std::string LUI(uint32_t Instruction);
	std::string LW(uint32_t Instruction);
	std::string LWCz(uint32_t Instruction);
	std::string LWL(uint32_t Instruction);
	std::string LWR(uint32_t Instruction);
	std::string MFCz(uint32_t Instruction);
	std::string MFHI(uint32_t Instruction);
	std::string MFLO(uint32_t Instruction);
	std::string MTCz(uint32_t Instruction);
	std::string MTHI(uint32_t Instruction);
	std::string MTLO(uint32_t Instruction);
	std::string MULT(uint32_t Instruction);
	std::string MULTU(uint32_t Instruction);
	std::string NOR(uint32_t Instruction);
	std::string OR(uint32_t Instruction);
	std::string ORI(uint32_t Instruction);
	std::string RFE(uint32_t Instruction);
	std::string SB(uint32_t Instruction);
	std::string SH(uint32_t Instruction);
	std::string SLL(uint32_t Instruction);
	std::string SLLV(uint32_t Instruction);
	std::string SLT(uint32_t Instruction);
	std::string SLTI(uint32_t Instruction);
	std::string SLTIU(uint32_t Instruction);
	std::string SLTU(uint32_t Instruction);
	std::string SRA(uint32_t Instruction);
	std::string SRAV(uint32_t Instruction);
	std::string SRL(uint32_t Instruction);
	std::string SRLV(uint32_t Instruction);
	std::string SUB(uint32_t Instruction);
	std::string SUBU(uint32_t Instruction);
	std::string SW(uint32_t Instruction);
	std::string SWCz(uint32_t Instruction);
	std::string SWL(uint32_t Instruction);
	std::string SWR(uint32_t Instruction);
	std::string SYSCALL(uint32_t Instruction);
	std::string XOR(uint32_t Instruction);
	std::string XORI(uint32_t Instruction);


	const RegImmTable const RegImmPtr[18] = {
		&Disasm::BLTZ, &Disasm::BGEZ, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		&Disasm::BLTZAL, &Disasm::BGEZAL
	};

	const RtypeTable const RtypePtr[64] = {
		&Disasm::SLL, nullptr, &Disasm::SRL, &Disasm::SRA, &Disasm::SLLV, nullptr, &Disasm::SRLV, &Disasm::SRAV,
		&Disasm::JR, &Disasm::JALR, nullptr, nullptr, &Disasm::SYSCALL, &Disasm::BREAK, nullptr, nullptr,
		&Disasm::MFHI, &Disasm::MTHI, &Disasm::MFLO, &Disasm::MTLO, nullptr, nullptr, nullptr, nullptr,
		&Disasm::MULT, &Disasm::MULTU, &Disasm::DIV, &Disasm::DIVU, nullptr, nullptr, nullptr, nullptr,
		&Disasm::ADD, &Disasm::ADDU, &Disasm::SUB, &Disasm::SUBU, &Disasm::AND, &Disasm::OR, &Disasm::XOR, &Disasm::NOR,
		nullptr, nullptr, &Disasm::SLT, &Disasm::SLTU, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
	};

	const OpcodeTable const OpcodePtr[64] = {
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

