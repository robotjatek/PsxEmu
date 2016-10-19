#pragma once
#include <cstdint>
#include "ICoprocessor.h"

class Cop0 : public ICoprocessor
{
private:
	enum StatusRegisterFields
	{
		CU = 0xF0000000, //coprocessor usuable
		RE = 0x2000000, //reverse endianness
		BEV = 0x400000, //boot exception vectors;  when BEV == 1, the CPU uses the ROM (kseg1)space exception entry point(described in a later chapter). BEV is usually set to zero in running systems; this relocates the exception  vectors.to  RAM  addresses, speeding  accesses  and allowing the use of “user supplied” exception service routines.
		TS = 0x200000, //TLB shutdown - If set to 1, the processor is dead in the water and needs to be reset.
		PE = 0x100000, //cache parity error
		CM = 0x80000, //Set to 1 if the most recent data cache load missed, but only if IsC is set.
		PZ = 0x40000, //If set to 1, uses space parity for outgoing data.
		SvC = 0x20000, //If set, the cache control lines affect the instruction cache rather than the data cache.
		IsC = 0x10000, // If set, the data cache is detached from main memory.
		IntMask = 0xFF00, //While these bits are set, the corresponding interrupts are masked and do not cause interrupt exceptions.
		KUo = 0x20, //Kernel/User old. This is the privilege state two exceptions previously. A ‘0’ indicates kernel mode.
		IEo = 0x10, // Interrupt Enable old.This is the global interrupt enable state two exceptions previously.A ‘1’ indicates that interrupts were enabled, 	subject to the IM mask.
		KUp = 0x8, //Previous kernel/user mode bit (1 = user mode)
		IEp = 0x4, //Previous interrupt enable bit (0 = mask all interrupts)
		KUc = 0x2, //current user/kernel mode
		IEc = 0x1, //current interrupt enable

		KernelModeBits = 0x2A,
		InterruptBits = 0x15,
	};

	enum CauseRegisterFields
	{
		BranchDelay			= 0x80000000,	//bd is set (1), if the last exception was taken while the processor was executing in the branch delay slot. If so, then the EPC will be rolled back to point to the branch instruction, so that it can be re - executed and the branch direction re - determined.
		CoprocessorError	= 0x30000000,	//The Co-processor Error field captures the co-processor unit number	referenced when a Co - processor Unusable exception is detected.
		InterruptsPending	= 0xFC00,		//The Interrupt Pending field indicates which interrupts are pending. Regardless of which interrupts are masked, the IP field can be used to determine which interrupts are pending.
		SoftwareInterrupts	= 0x300,		//The Software interrupt bits can be thought of as the logical extension of the IP field.The SW interrupts can be written to to force an interrupt to be pending to the processor, and are useful in the prioritization of exceptions.To set a software interrupt, a “1” is written to the appropriate SW bit, and a “0” will clear the pending interrupt.There are corresponding interrupt mask bits in the status register for these interrupts.
		ExceptionCode		= 0x7C			//The exception code field indicates the reason for the last exception.
	};

	enum RegisterNames
	{
		Index					= 0,
		Random					= 1,
		TLB_LO					= 2,
		Context					= 4,  //exception handler
		Bad_Virtual_Address		= 8,  //exception handler - The Bad VAddr register saves the entire bad virtual address for any addressing exception.
		TLB_HI					= 10,
		Status					= 12, //exception handler
		Cause					= 13, //exception handler - The contents of the Cause register describe the last exception
		Exception_PC			= 14, //exception handler - The 32-bit EPC register contains the virtual address of the instruction which took the exception, from which point processing resumes after the exception	has been serviced.When the virtual address of the instruction resides in a	branch delay slot, the EPC contains the virtual address of the instruction	immediately preceding the exception(that is, the EPC points to the Branch or Jump instruction).
		Pr_id					= 15
	};
	uint32_t cop_registers[16];

public:
	enum ExceptionCodes
	{
		Int = 0,	//External interrupt
		MOD = 1,	//TLB Modification exception
		TLBL = 2,	//TLB miss exception (load/fetch)
		TLBS = 3,	//TLB miss exception (store)
		AdEL = 4,	//Address error exception (load/fetch)
		AdES = 5,	//Address error exception (store)
		IBE = 6,	//Bus error exception (fetch)
		DBE = 7,	//Bus error exception (load/store)
		Sys = 8,	//Syscall exception
		Bp = 9,		//Breakpoint exception
		RI = 10,	//Reserved instruction exception
		CpU = 11,	//Coprocessor unusuable exception
		Ovf = 12	//Arithmetic overflow exception
	};

	Cop0();
	virtual ~Cop0();
	void setException(uint32_t pc, ExceptionCodes exceptioncode, bool branch_delay);
	void enableStatusBits(uint32_t bits);
	void disableStatusBits(uint32_t bits);
	void setStatusRegister(uint32_t st);
	void pushKernelBitAndInterruptBit(bool user_mode, bool interrupt_enable);
	void pushInterruptBit(bool interrupt_enable);
	virtual void Operation(uint32_t cop_fun);
	virtual void LoadWord(uint32_t w, uint8_t rt);
	virtual uint32_t GetWord(uint8_t rt);
};