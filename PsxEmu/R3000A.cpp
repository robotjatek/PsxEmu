#include "R3000A.h"
#include "Cop0.h"
#include "Memory.h"
#include <cstring>
//TODO: sign extend f�ggv�ny meg�r�sa �s megh�v�sa minden signed mem�rahozz�f�r�shez

void R3000A::Step()
{
	HandleIRQRequests();
	Decode(Fetch());
}

void R3000A::StartLogging()
{
	disasm->Enable();
}

void R3000A::StopLogging()
{
	disasm->Disable();
	InstructionLogger.close();
}

inline uint32_t R3000A::Fetch()
{
	//Data stored as little endian, so for easy decoding it is better to reverse byte order
	/*uint32_t t1, t2, t3, t4;
	t1 = m_memory.read(pc);
	t2 = m_memory.read(pc + 1);
	t3 = m_memory.read(pc + 2);
	t4 = m_memory.read(pc + 3);
	pc += 4;

	return ((t4 << 24) | (t3 << 16) | (t2 << 8) | t1);*/
	uint32_t ret = m_memory->Read<uint32_t>(pc);
	pc += 4;
	return ret;
}

inline void R3000A::Decode(uint32_t instruction_word)
{
	delay_slot = false;
	
	if (disasm->IsEnabled())
	{
		InstructionLogger << std::hex << pc - 4 << " " << disasm->DecodeInstruction(instruction_word) << "\n";
	}
	//TODO: execute decoded instruction
	uint8_t opcode = (instruction_word & 0xFC000000) >> 26;
	if (opcode == 0) //opcode = 0
	{
		//rtype instruction
		RTypeInstruction r = getRTypeFields(instruction_word);
		if ( r.funct == 0 //SLL
			|| r.funct == 2 //SRL
			|| r.funct == 3) //SRA)
		{
			(this->*rtypes[r.funct])(r.rd, r.sht, r.rt);
		}
		else
		{
			(this->*rtypes[r.funct])(r.rd, r.rs, r.rt);
		}
	}
	else if (opcode == 2 || opcode == 3)
	{
		JTypeInstruction j = getJTypeFields(instruction_word);
		(this->*jtypes[j.op])(j.target);
	}
	else if ((opcode & ~0x3) == 0x10)
	{
		//COPz
		uint8_t copnum = opcode & 0x3;
		JTypeInstruction j = getJTypeFields(instruction_word);

		if (j.target & 0x2000000)
		{
			if (copnum == 0 && ((j.target & 0x3f) == 0x10)) //rfe instruction
			{
				m_cop0->ReturnFromInterrupt();
				exception_pending = false;
			}
			else
			{
				m_copx[copnum]->Operation(j.target&~(0x2000000));
			}
		}
		else
		{
			RTypeInstruction r = getRTypeFields(instruction_word);

			if (r.rs == 0)
			{
				//mfcz
				write_register(r.rt, m_copx[copnum]->MoveFromCoprocessor(r.rd));
			}
			else if (r.rs == 0x04)
			{
				//mtcz
				m_copx[copnum]->MoveToCoprocessor(r.rd, read_register(r.rt));
			}
			else if (r.rs == 0x02)
			{
				//cfcz
				write_register(r.rt, m_copx[copnum]->MoveControlFromCoprocessor(r.rd));
			}
			else if (r.rs == 0x06)
			{
				//ctcz
				m_copx[copnum]->MoveControlToCoprocessor(r.rd, read_register(r.rt));
			}
		}
	}
	else if ((opcode & ~0x3) == 0x30)
	{
		//LWCz
		uint8_t copnum = opcode & 0x3;
		ITypeInstruction i = getITypeFields(instruction_word);
		int16_t offset = i.immediate;
		uint32_t w = m_memory->Read<uint32_t>(read_register(i.rs) + offset);
		m_copx[copnum]->LoadWord(w, i.rt);
	}
	else if ((opcode & ~0x03) == 0x38)
	{
		//SWCz
		uint8_t copnum = opcode & 0x3;
		ITypeInstruction i = getITypeFields(instruction_word);
		int16_t offset = i.immediate;

		m_memory->Write<uint32_t>(read_register(i.rs) + offset, m_copx[copnum]->GetWord(i.rt));
	}
	else
	{
		ITypeInstruction i = getITypeFields(instruction_word);
		(this->*itypes[i.op])(i.rt, i.rs, i.immediate);
	}


	instruction_counter++;

	//handle delay slot after a branch, load or jump
	if (delay_slot)
	{
		Decode(Fetch());
		
		//if (!exception_pending) //check if the instruction is still in the delay slot, because interrupts roll back the pc to the previous branch instruction
		{
			pc = delay_slot_address;
		}

	}
}

inline void R3000A::write_register(uint8_t reg, uint32_t data)
{
	reg &= 0x1F;
	if (reg == 0)
	{
		registers[reg] = 0;
	}
	else
	{
		registers[reg] = data;
	}
}

uint32_t R3000A::read_register(uint8_t reg) const
{
	reg &= 0x1F;
	if (reg == 0)
	{
		return 0;
	}
	else
	{
		return registers[reg];
	}
}

struct RTypeInstruction R3000A::getRTypeFields(uint32_t opcode)
{
	struct RTypeInstruction ret;
	ret.op = (opcode & 0xFC000000) >> 26;
	ret.rs = (opcode & 0x3E00000) >> 21;
	ret.rt = (opcode & 0x1F0000) >> 16;
	ret.rd = (opcode & 0xF800) >> 11;
	ret.sht = (opcode & 0x7C0) >> 6;
	ret.funct = (opcode & 0x3F);
	return ret;
}

struct ITypeInstruction R3000A::getITypeFields(uint32_t opcode)
{
	struct ITypeInstruction ret;
	ret.op = (opcode & 0xFC000000) >> 26;
	ret.rs = (opcode & 0x3E00000) >> 21;
	ret.rt = (opcode & 0x1F0000) >> 16;
	ret.immediate = (opcode & 0xFFFF);
	return ret;
}

struct JTypeInstruction R3000A::getJTypeFields(uint32_t opcode)
{
	struct JTypeInstruction ret;
	ret.op = (opcode & 0xFC000000) >> 26;
	ret.target = (opcode & 0x3FFFFFF);
	return ret;
}

inline void R3000A::RtypeNull(uint8_t, uint8_t, uint8_t)
{
	RTypeInstruction r = getRTypeFields(m_memory->Read<uint32_t>(pc - 4));
	Null(r.op, r.funct);
}

inline void R3000A::ItypeNull(uint8_t, uint8_t, uint16_t)
{
	ITypeInstruction i = getITypeFields(m_memory->Read<uint32_t>(pc - 4));
	Null(i.op, 0);
}

inline void R3000A::JtypeNull(uint32_t)
{
	JTypeInstruction j = getJTypeFields(m_memory->Read<uint32_t>(pc - 4));
	Null(j.op, 0);
}

inline void R3000A::Null(uint8_t op, uint8_t funct)
{
	LOG_FATAL << "Unrecognized opcode: " << (int)op << "funct: " << (int)funct << "pc: " << std::hex << pc;
	this->is_running = false;
}

inline void R3000A::CallRegimmFunc(uint8_t rt, uint8_t rs, uint16_t imm)
{
	switch (rt)
	{
	case 0x1:
		Bgez(rt, rs, imm);
		break;
	case 0x0:
		Bltz(rt, rs, imm);
		break;
	case 0x11:
		Bgezal(rt, rs, imm);
		break;
	case 0x10:
		Bltzal(rt, rs, imm);
		break;
	default:
		break;
	}
}

inline void R3000A::Add(uint8_t rd, uint8_t rs, uint8_t rt)
{
	//TODO: signed �sszead�s?
	uint32_t _rs = read_register(rs);
	uint32_t _rt = read_register(rt);
	uint32_t temp = _rs + _rt;

	if ((~(_rs ^ _rt))&(_rs ^ temp) & 0x80000000)
	{
		pc = m_cop0->setException(pc - 4, Cop0::ExceptionCodes::Ovf, this->delay_slot);
		this->exception_pending = true;
		this->delay_slot = false;
	}
	else
	{
		write_register(rd, temp);
	}
}

inline void R3000A::Addi(uint8_t rt, uint8_t rs, uint16_t imm)
{
	uint32_t _rs = read_register(rs);
	int16_t _imm = imm;
	uint32_t temp = _rs + _imm;
	if ((~(_rs ^ _imm))&(_rs ^ temp) & 0x80000000)
	{
		pc = m_cop0->setException(pc - 4, Cop0::ExceptionCodes::Ovf, this->delay_slot);
		this->exception_pending = true;
		this->delay_slot = false;
	}
	else
	{
		write_register(rt, temp);
	}
}

inline void R3000A::Addiu(uint8_t rt, uint8_t rs, uint16_t imm)
{
	uint32_t _rs = read_register(rs);
	int16_t _imm = imm;
	write_register(rt, _rs + _imm);
}

inline void R3000A::Addu(uint8_t rd, uint8_t rs, uint8_t rt)
{
	uint32_t _rs = read_register(rs);
	uint32_t _rt = read_register(rt);
	write_register(rd, _rs + _rt);
}

inline void R3000A::And(uint8_t rd, uint8_t rs, uint8_t rt)
{
	uint32_t _rs = read_register(rs);
	uint32_t _rt = read_register(rt);
	write_register(rd, _rs & _rt);
}

inline void R3000A::Andi(uint8_t rt, uint8_t rs, uint16_t imm)
{
	uint32_t _rs = read_register(rs);
	write_register(rt, _rs & imm);
}

inline void R3000A::Beq(uint8_t rt, uint8_t rs, uint16_t imm)
{
	if (read_register(rs) == read_register(rt))
	{
		int32_t offset = (int16_t)imm;
		offset *= 4;
		delay_slot_address = pc + offset;
		delay_slot = true;
	}

}

inline void R3000A::Bgez(uint8_t, uint8_t rs, uint16_t offset)
{
	int32_t _rs = (int32_t)read_register(rs);
	if (_rs >= 0)
	{
		int32_t t_offset = (int16_t)offset;
		t_offset *= 4;
		delay_slot_address = pc + t_offset;
		delay_slot = true;
	}
}

inline void R3000A::Bgezal(uint8_t, uint8_t rs, uint16_t offset)
{
	write_register(31, pc + 4); //PC is already pointing to the next instruction, so here it should be increased by 4 and not 8 bytes, which is the second instruction after the branch (probably)
	int32_t _rs = (int32_t)read_register(rs);
	if (_rs >= 0)
	{
		int32_t t_offset = (int16_t)offset;
		t_offset *= 4;
		delay_slot_address = pc + t_offset;
		delay_slot = true;
	}
}

inline void R3000A::Bgtz(uint8_t, uint8_t rs, uint16_t imm)
{
	int32_t _rs = (int32_t)read_register(rs);
	if (_rs > 0)
	{
		int32_t offset = (int16_t)imm;
		offset *= 4;
		delay_slot_address = pc + offset;
		delay_slot = true;
	}
}

inline void R3000A::Blez(uint8_t, uint8_t rs, uint16_t imm)
{
	int32_t _rs = (int32_t)read_register(rs);
	if (_rs <= 0)
	{
		int32_t offset = (int16_t)imm;
		offset *= 4;
		delay_slot_address = pc + offset;
		delay_slot = true;
	}
}

inline void R3000A::Bltz(uint8_t, uint8_t rs, uint16_t imm)
{
	int32_t _rs = (int32_t)read_register(rs);
	if (_rs < 0)
	{
		int32_t offset = (int16_t)imm;
		offset *= 4;
		delay_slot_address = pc + offset;
		delay_slot = true;
	}
}

inline void R3000A::Bltzal(uint8_t, uint8_t rs, uint16_t imm)
{
	write_register(31, pc + 4); //PC is already pointing to the next instruction, so here it should be increased by 4 and not 8 bytes, which is the second instruction after the branch (probably)
	int32_t _rs = (int32_t)read_register(rs);
	if (_rs < 0)
	{
		int32_t t_offset = (int16_t)imm;
		t_offset *= 4;
		delay_slot_address = pc + t_offset;
		delay_slot = true;
	}
}

inline void R3000A::Bne(uint8_t rt, uint8_t rs, uint16_t imm)
{
	if (read_register(rs) != read_register(rt))
	{
		int32_t offset = (int16_t)imm;
		offset *= 4;
		delay_slot_address = pc + offset;
		delay_slot = true;
	}
}

inline void R3000A::Break(uint8_t, uint8_t, uint8_t)
{
	pc = this->m_cop0->setException(this->pc - 4, Cop0::ExceptionCodes::Bp, this->delay_slot);
	this->exception_pending = true;
	this->delay_slot = false;
}

inline void R3000A::Cfcz(uint8_t, uint8_t, uint8_t)
{
}

inline void R3000A::Copz(uint32_t)
{
}

inline void R3000A::Ctcz(uint8_t, uint8_t, uint8_t)
{
}

inline void R3000A::Div(uint8_t, uint8_t rs, uint8_t rt)
{
	//TODO: 36 cycles
	int32_t _rs = read_register(rs);
	int32_t _rt = read_register(rt);
	if (_rt > 0)
	{
		lo = _rs / _rt;
		hi = _rs % _rt;
	}
	else if (_rs == 0x80000000 && _rt == 0xffffffff)
	{
		lo = 0x80000000;
		hi = 0;
	}
	else
	{
		//from PSX-guide
		if (_rs >= 0)
		{
			lo = 0xffffffff;
			hi = _rs;
		}
		else
		{
			lo = 1;
			hi = _rs;
		}
		//TODO: check if this is correct
		//XXX: number/0 = +inf && 0/0=NaN (????)
		//XXX: from the docs: if the divisior in rt is zero, the aritmetic result is undefined
	}
}

inline void R3000A::Divu(uint8_t, uint8_t rs, uint8_t rt)
{
	uint32_t _rs = read_register(rs);
	uint32_t _rt = read_register(rt);
	if (_rt)
	{
		lo = _rs / _rt;
		hi = _rs % _rt;
	}
	else
	{
		lo = 0xffffffff;
		hi = _rs;
	}

}

inline void R3000A::J(uint32_t target)
{
	//pc is already on the next instruction
	delay_slot_address = (pc & 0xf0000000) | (target << 2);
	delay_slot = true;
}

inline void R3000A::Jal(uint32_t target)
{
	write_register(31, pc + 4);
	delay_slot_address = (pc & 0xf0000000) | (target << 2);
	delay_slot = true;
}

inline void R3000A::Jalr(uint8_t rd, uint8_t rs, uint8_t)
{
	write_register(rd, pc + 4);
	delay_slot_address = read_register(rs);
	delay_slot = true;
	//TODO: The effective target address in GPR rs must be naturally aligned. If either of the two	least - significant bits are not - zero, then an Address Error exception occurs, not for the jump instruction, but when the branch target is subsequently fetched as an instruction.
}

inline void R3000A::Jr(uint8_t, uint8_t rs, uint8_t)
{
	delay_slot_address = read_register(rs);
	delay_slot = true;
	//TODO: The effective target address in GPR rs must be naturally aligned. If either of the two	least - significant bits are not - zero, then an Address Error exception occurs, not for the jump instruction, but when the branch target is subsequently fetched as an instruction.
}

inline void R3000A::Lb(uint8_t rt, uint8_t base, uint16_t offset)
{
	//TODO: address error exception? 
	int16_t signed_offset = offset;
	int32_t sign_extended_value = (int8_t)m_memory->Read<uint8_t>(read_register(base) + signed_offset);
	write_register(rt, sign_extended_value);
}

inline void R3000A::Lbu(uint8_t rt, uint8_t base, uint16_t offset)
{
	//TODO: address error exception? 
	int16_t signed_offset = offset;
	write_register(rt, m_memory->Read<uint8_t>(read_register(base) + signed_offset));
}

inline void R3000A::Lh(uint8_t rt, uint8_t base, uint16_t offset)
{
	//TODO: address error exception? 
	int16_t signed_offset = offset;
	int32_t sign_extended_value = (int16_t)m_memory->Read<uint16_t>(read_register(base) + signed_offset);
	write_register(rt, sign_extended_value);
}

inline void R3000A::Lhu(uint8_t rt, uint8_t base, uint16_t offset)
{
	//TODO: address error exception? 
	int16_t signed_offset = offset;
	write_register(rt, m_memory->Read<uint16_t>(read_register(base) + signed_offset));
}

inline void R3000A::Lui(uint8_t rt, uint8_t, uint16_t imm)
{
	//TODO: check LUI instruction!!!!
	uint32_t t = ((uint32_t)imm) << 16;
	write_register(rt, t);
}

inline void R3000A::Lw(uint8_t rt, uint8_t base, uint16_t offset)
{
	//TODO: address error exception? 
	//TODO: isolate cache
	if (!m_cop0->GetStatusRegisterBit(Cop0::StatusRegisterFields::IsC))
	{
		int16_t signed_offset = offset;
		write_register(rt, m_memory->Read<uint32_t>(
			read_register(base) + signed_offset));
	}
	else
	{
		LOG_INFO << "Cache is isolated";
	}
}

inline void R3000A::Lwcz(uint8_t, uint8_t, uint16_t)
{
}

inline void R3000A::Lwl(uint8_t rt, uint8_t base, uint16_t offset)
{
	int16_t signed_offset = offset;
	uint32_t w = read_register(rt);
	uint32_t t = m_memory->Read<uint16_t>(read_register(base) + signed_offset);
	w &= 0xFFFF0000;
	w |= t << 16;
	write_register(rt, w);
}

inline void R3000A::Lwr(uint8_t rt, uint8_t base, uint16_t offset)
{
	int16_t signed_offset = offset;
	uint32_t w = read_register(rt);
	uint32_t t = m_memory->Read<uint16_t>(read_register(base) + signed_offset);
	w &= 0x0000FFFF;
	w |= t;
	write_register(rt, w);
}

inline void R3000A::Mfcz(uint8_t, uint8_t, uint8_t)
{
}

inline void R3000A::Mfhi(uint8_t rd, uint8_t, uint8_t)
{
	write_register(rd, hi);
}

inline void R3000A::Mflo(uint8_t rd, uint8_t, uint8_t)
{
	write_register(rd, lo);
}

inline void R3000A::Mtcz(uint8_t, uint8_t, uint8_t)
{
}

inline void R3000A::Mthi(uint8_t, uint8_t rs, uint8_t)
{
	hi = read_register(rs);
}

inline void R3000A::Mtlo(uint8_t, uint8_t rs, uint8_t)
{
	lo = read_register(rs);
}

inline void R3000A::Mult(uint8_t, uint8_t rs, uint8_t rt)
{
	int64_t result = ((int32_t)read_register(rs)) * ((int32_t)read_register(rt));
	lo = result & 0x00000000ffffffff;
	hi = (result & 0xffffffff00000000) >> 32;
}

inline void R3000A::Multu(uint8_t, uint8_t rs, uint8_t rt)
{
	uint64_t result = read_register(rs) * read_register(rt);
	lo = result & 0x00000000ffffffff;
	hi = (result & 0xffffffff00000000) >> 32;
}

inline void R3000A::Nor(uint8_t rd, uint8_t rs, uint8_t rt)
{
	write_register(rd, ~(read_register(rs) | read_register(rt)));
}

inline void R3000A::Or(uint8_t rd, uint8_t rs, uint8_t rt)
{
	write_register(rd, read_register(rs) | read_register(rt));
}

inline void R3000A::Ori(uint8_t rt, uint8_t rs, uint16_t imm)
{
	write_register(rt, read_register(rs) | imm);
}

inline void R3000A::Rfe(uint8_t, uint8_t, uint8_t)
{
}

inline void R3000A::Sb(uint8_t rt, uint8_t base, uint16_t offset)
{
	//todo: address error
	if (!m_cop0->GetStatusRegisterBit(Cop0::StatusRegisterFields::IsC))
	{
		int32_t signed_offset = (int16_t)offset;
		m_memory->Write<uint8_t>(read_register(base)
			+ signed_offset,
			(uint8_t)read_register(rt));
	}
	else
	{
		LOG_INFO << "Cache is isolated";
		//todo: implement cache
	}
}

inline void R3000A::Sh(uint8_t rt, uint8_t base, uint16_t offset)
{
	//todo: address error
	if (!m_cop0->GetStatusRegisterBit(Cop0::StatusRegisterFields::IsC))
	{
		int16_t signed_offset = offset;
		m_memory->Write<uint16_t>(read_register(base) + signed_offset, (uint16_t)read_register(rt));
	}
	else
	{
		LOG_INFO << "Cache is isolated";
		//todo: implement cache
	}
}

inline void R3000A::Sll(uint8_t rd, uint8_t rs, uint8_t rt)
{
	write_register(rd, read_register(rt) << rs);
}

inline void R3000A::Sllv(uint8_t rd, uint8_t rs, uint8_t rt)
{
	write_register(rd, read_register(rt) << (read_register(rs) & 0x1f));
}

inline void R3000A::Slt(uint8_t rd, uint8_t rs, uint8_t rt)
{
	int32_t _rs = read_register(rs);
	int32_t _rt = read_register(rt);
	write_register(rd, _rs < _rt);
}

inline void R3000A::Slti(uint8_t rt, uint8_t rs, uint16_t imm)
{
	int32_t _rs = (int32_t)read_register(rs);
	int32_t _imm = (int16_t)imm;
	write_register(rt, _rs < _imm ? 1 : 0);
}

inline void R3000A::Sltiu(uint8_t rt, uint8_t rs, uint16_t imm)
{
	write_register(rt, read_register(rs) < imm ? 1 : 0);
}

inline void R3000A::Sltu(uint8_t rd, uint8_t rs, uint8_t rt)
{
	uint32_t _rs = read_register(rs);
	uint32_t _rt = read_register(rt);
	write_register(rd, _rs < _rt);
}

inline void R3000A::Sra(uint8_t rd, uint8_t rs, uint8_t rt)
{
	write_register(rd, ((int32_t)read_register(rt)) >> rs);
}

inline void R3000A::Srav(uint8_t rd, uint8_t rs, uint8_t rt)
{
	write_register(rd, ((int32_t)read_register(rt)) >> (read_register(rs) & 0x1f));
}

inline void R3000A::Srl(uint8_t rd, uint8_t rs, uint8_t rt)
{
	write_register(rd, (read_register(rt) >> rs));
}

inline void R3000A::Srlv(uint8_t rd, uint8_t rs, uint8_t rt)
{
	write_register(rd, read_register(rt) >> (read_register(rs) & 0x1f));
}

inline void R3000A::Sub(uint8_t rd, uint8_t rs, uint8_t rt)
{
	int32_t _rs = (int32_t)read_register(rs);
	int32_t _rt = (int32_t)read_register(rt);
	int32_t result = _rs - _rt;
	if ((~(_rs ^ _rt))&(_rs ^ result) & 0x80000000)
	{
		pc = m_cop0->setException(pc - 4, Cop0::ExceptionCodes::Ovf, this->delay_slot);
		this->exception_pending = true;
		this->delay_slot = false;
	}
	else
	{
		write_register(rd, result);
	}
}

inline void R3000A::Subu(uint8_t rd, uint8_t rs, uint8_t rt)
{
	uint32_t _rs = read_register(rs);
	uint32_t _rt = read_register(rt);
	uint32_t result = _rs - _rt;
	write_register(rd, result);
}

inline void R3000A::Sw(uint8_t rt, uint8_t base, uint16_t offset)
{
	//todo:: address error
	if (!m_cop0->GetStatusRegisterBit(Cop0::StatusRegisterFields::IsC))
	{
		int32_t _offset = (int16_t)offset;
		m_memory->Write<uint32_t>(read_register(base) + _offset, read_register(rt));
	}
	//else
	{
		//std::cout << "Cache is isolated" << std::endl;
		//todo: implement cache
	}
}

inline void R3000A::Swcz(uint8_t, uint8_t, uint16_t)
{
}

inline void R3000A::Swl(uint8_t rt, uint8_t base, uint16_t offset)
{
	int16_t signed_offset = offset;
	uint32_t w = read_register(rt);
	w &= 0xFFFF0000;
	w = w >> 16;
	m_memory->Write<uint16_t>(read_register(base) + signed_offset, (uint16_t)w);
}

inline void R3000A::Swr(uint8_t rt, uint8_t base, uint16_t offset)
{
	int16_t signed_offset = offset;
	uint32_t w = read_register(rt);
	w &= 0x0000FFFF;
	m_memory->Write<uint16_t>(read_register(base) + signed_offset, (uint16_t)w);
}

inline void R3000A::Syscall(uint8_t, uint8_t, uint8_t)
{
	pc = m_cop0->setException(this->pc - 4, Cop0::ExceptionCodes::Sys, this->delay_slot);
	this->exception_pending = true;
	this->delay_slot = false;
	LOG_DEBUG << "Syscall";
}

inline void R3000A::Xor(uint8_t rd, uint8_t rs, uint8_t rt)
{
	write_register(rd, read_register(rs) ^ read_register(rt));
}

inline void R3000A::Xori(uint8_t rt, uint8_t rs, uint16_t imm)
{
	write_register(rt, read_register(rs) ^ imm);
}

void R3000A::HandleIRQRequests()
{
	static uint32_t PrevIStat = 0; //IStat register is edge triggered, so the exception only occurs on change
	uint32_t IStat = m_memory->GetIStatField();
	if (IStat != PrevIStat)
	{
		uint32_t IMask = m_memory->GetIMaskField();
		if (IStat & IMask)
		{
			uint32_t ret = m_cop0->setException(this->pc - 4, Cop0::ExceptionCodes::Int, this->delay_slot); //??? TODO: use Int as exceptioncode?
			if (ret != pc)
			{
				pc = ret;
				this->exception_pending = true;
			}
			else
			{
				m_cop0->disableStatusBits(0x400); //from nocash specifications: "cop0r13.bit10 is NOT a latch, ie. it gets automatically cleared as soon as "(I_STAT AND I_MASK)=zero"
			}
		}
		PrevIStat = IStat;
	}
}

R3000A::R3000A(Memory* mem)
{
	m_memory = mem;
	this->pc = BIOS_START_UNCACHED; //reset vector - start of the BIOS: 0xbfc00000
	this->kernel_mode = false;
	this->exception_pending = false;
	this->delay_slot = false;
	memset(registers, 0, sizeof(registers));

	m_copx[0] = new Cop0();
	m_copx[1] = nullptr;
	m_copx[2] = nullptr;
	m_copx[3] = nullptr;

	m_cop0 = dynamic_cast<Cop0*>(m_copx[0]);

	for (int i = 0; i < 0x3F; i++)
	{
		rtypes[i] = &R3000A::RtypeNull;
		itypes[i] = &R3000A::ItypeNull;
		jtypes[i] = &R3000A::JtypeNull;
	}

	rtypes[0x20] = &R3000A::Add;
	rtypes[0x21] = &R3000A::Addu;
	rtypes[0x24] = &R3000A::And;
	rtypes[0x0d] = &R3000A::Break;
	rtypes[0x1a] = &R3000A::Div;
	rtypes[0x1b] = &R3000A::Divu;
	rtypes[0x09] = &R3000A::Jalr;
	rtypes[0x08] = &R3000A::Jr;
	rtypes[0x10] = &R3000A::Mfhi;
	rtypes[0x12] = &R3000A::Mflo;
	rtypes[0x11] = &R3000A::Mthi;
	rtypes[0x13] = &R3000A::Mtlo;
	rtypes[0x18] = &R3000A::Mult;
	rtypes[0x19] = &R3000A::Multu;
	rtypes[0x27] = &R3000A::Nor;
	rtypes[0x25] = &R3000A::Or;
	rtypes[0x00] = &R3000A::Sll;
	rtypes[0x04] = &R3000A::Sllv;
	rtypes[0x2a] = &R3000A::Slt;
	rtypes[0x2b] = &R3000A::Sltu;
	rtypes[0x03] = &R3000A::Sra;
	rtypes[0x07] = &R3000A::Srav;
	rtypes[0x02] = &R3000A::Srl;
	rtypes[0x06] = &R3000A::Srlv;
	rtypes[0x22] = &R3000A::Sub;
	rtypes[0x23] = &R3000A::Subu;
	rtypes[0x0c] = &R3000A::Syscall;
	rtypes[0x26] = &R3000A::Xor;

	itypes[0x08] = &R3000A::Addi;
	itypes[0x09] = &R3000A::Addiu;
	itypes[0x0c] = &R3000A::Andi;
	itypes[0x04] = &R3000A::Beq;
	itypes[0x01] = &R3000A::CallRegimmFunc; //regimm type functions with opcode 0b000001
	itypes[0x07] = &R3000A::Bgtz;
	itypes[0x06] = &R3000A::Blez;
	itypes[0x05] = &R3000A::Bne;
	itypes[0x20] = &R3000A::Lb;
	itypes[0x24] = &R3000A::Lbu;
	itypes[0x21] = &R3000A::Lh;
	itypes[0x25] = &R3000A::Lhu;
	itypes[0x0f] = &R3000A::Lui;
	itypes[0x23] = &R3000A::Lw;
	itypes[0x22] = &R3000A::Lwl;
	itypes[0x26] = &R3000A::Lwr;
	itypes[0x0d] = &R3000A::Ori;
	itypes[0x28] = &R3000A::Sb;
	itypes[0x29] = &R3000A::Sh;
	itypes[0x0a] = &R3000A::Slti;
	itypes[0x0b] = &R3000A::Sltiu;
	itypes[0x2b] = &R3000A::Sw;
	itypes[0x2a] = &R3000A::Swl;
	itypes[0x2e] = &R3000A::Swr;
	itypes[0x0e] = &R3000A::Xori;

	jtypes[0x02] = &R3000A::J;
	jtypes[0x03] = &R3000A::Jal;

	InstructionLogger.open("InstructionLog.txt", std::ios::out);
	disasm = new Disasm(m_memory);
	instruction_counter = 0;
}

R3000A::~R3000A()
{
	for (int i = 0; i < 4; i++)
	{
		delete m_copx[i];
	}
	InstructionLogger.close();
	delete disasm;
}

void R3000A::Run()
{
	is_running = true;
	while (is_running)
	{
		Step();
	}
}

void R3000A::Stop()
{
	is_running = false;
}

