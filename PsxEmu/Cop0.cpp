#include "Cop0.h"
#include <iostream>


Cop0::Cop0()
{
	this->cop_registers[RegisterNames::Status] = 0;
	enableStatusBits(StatusRegisterFields::BEV | StatusRegisterFields::TS);
}


Cop0::~Cop0()
{
}

uint32_t Cop0::setException(uint32_t pc, ExceptionCodes exceptioncode, bool branch_delay, uint8_t coprocessor_error, uint32_t badvaddr)
{
	//todo: interrupt/exception mask
	std::cout << "Exception occured" << std::endl;
	pushKernelBitAndInterruptBit();

	cop_registers[RegisterNames::Cause] &= ~CauseRegisterFields::ExceptionCode;
	cop_registers[RegisterNames::Cause] |= (exceptioncode << 2);

	if (branch_delay)
	{
		cop_registers[RegisterNames::Cause] |= CauseRegisterFields::BranchDelay;
		cop_registers[RegisterNames::Exception_PC] = pc - 4; //TODO: -4 or -8?
	}
	else
	{
		cop_registers[RegisterNames::Cause] &= ~CauseRegisterFields::BranchDelay;
		cop_registers[RegisterNames::Exception_PC] = pc;
	}

	if (exceptioncode == ExceptionCodes::AdEL || exceptioncode == ExceptionCodes::AdES)
	{
		cop_registers[RegisterNames::Bad_Virtual_Address] = badvaddr;
	}
	else if (exceptioncode == ExceptionCodes::CpU)
	{
		cop_registers[RegisterNames::Cause] &= ~CauseRegisterFields::CoprocessorError;
		cop_registers[RegisterNames::Cause] |= (((uint32_t)coprocessor_error) << 28);
	}

	return cop_registers[RegisterNames::Status] & StatusRegisterFields::BEV ? GENERAL_EXCEPTION_BEV_1_ADDRESS : GENERAL_EXCEPTION_BEV_0_ADDRESS;
}

void Cop0::enableStatusBits(uint32_t bits)
{
	this->cop_registers[RegisterNames::Status] |= bits;
}

void Cop0::disableStatusBits(uint32_t bits)
{
	this->cop_registers[RegisterNames::Status] &= (~bits);
}

void Cop0::setStatusRegister(uint32_t st)
{
	this->cop_registers[RegisterNames::Status] = st;
}

void Cop0::pushKernelBitAndInterruptBit(bool user_mode, bool interrupt_enable)
{
	uint8_t bits = this->cop_registers[RegisterNames::Status] & (StatusRegisterFields::KernelModeBits | StatusRegisterFields::InterruptBits);
	bits <<= 2;
	bits &= 0x3F; //reset the two most significant bits as they are not needed
	if (user_mode)
	{
		bits |= 0x2; //set the current user mode bit to 1
	}

	if (interrupt_enable)
	{
		bits |= 0x1; //set the current interrupt enable bit to 1
	}

	this->cop_registers[RegisterNames::Status] &= ~(StatusRegisterFields::KernelModeBits | StatusRegisterFields::InterruptBits);
	this->cop_registers[RegisterNames::Status] |= bits;
}

void Cop0::popKernelBitAndInterruptBit()
{
	uint8_t bits = this->cop_registers[RegisterNames::Status] & (StatusRegisterFields::KernelModeBits | StatusRegisterFields::InterruptBits);
	uint8_t saved_KUo_IEo = bits & 0x30;
	bits >>= 2;
	bits |= saved_KUo_IEo;
	bits &= 0x3F; //reset the two most significant bits as they are not needed

	this->cop_registers[RegisterNames::Status] &= ~StatusRegisterFields::InterruptBits;
	this->cop_registers[RegisterNames::Status] |= bits;
}

void Cop0::pushInterruptBit(bool interrupt_enable)
{
	uint8_t bits = this->cop_registers[RegisterNames::Status] & StatusRegisterFields::InterruptBits;
	bits <<= 2;
	bits &= 0x3F; //reset the two most significant bits as they are not needed
	if (interrupt_enable)
	{
		bits |= 0x1; //set the current interrupt enable bit to 1
	}
	this->cop_registers[RegisterNames::Status] &= ~StatusRegisterFields::InterruptBits;
	this->cop_registers[RegisterNames::Status] |= bits;
}

void Cop0::Operation(uint32_t cop_fun)
{
	//TODO: implement cop0 operation
	throw 0;
}

void Cop0::LoadWord(uint32_t w, uint8_t rt)
{
	//TODO: implement cop0 loadword
	throw 0;
}

uint32_t Cop0::GetWord(uint8_t rt)
{
	//todo: implement cop0 getword
	return 0;
}

uint32_t Cop0::MoveControlFromCoprocessor(uint8_t rd)
{
	//unusuable on Cop0
	return uint32_t();
}

void Cop0::MoveControlToCoprocessor(uint8_t rd, uint32_t control)
{
	//unusuable on Cop0
}

uint32_t Cop0::MoveFromCoprocessor(uint8_t rd)
{
	return cop_registers[rd];
}

void Cop0::MoveToCoprocessor(uint8_t rd, uint32_t data)
{
	cop_registers[rd] = data;
}

void Cop0::ReturnFromInterrupt()
{
	popKernelBitAndInterruptBit();
}

bool Cop0::GetStatusRegisterBit(StatusRegisterFields f)
{
	return cop_registers[RegisterNames::Status] & f;
}
