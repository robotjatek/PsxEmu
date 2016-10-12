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

void Cop0::setException(uint32_t pc, ExceptionCodes exceptioncode, bool branch_delay)
{
	std::cout << "Exceptions are not implemented yet.";
	throw 0;
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
}
