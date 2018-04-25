#pragma once
#include <cstdint>
#include <fstream>
class Memory;

class Dma
{
public:
	typedef struct
	{
		bool ForceIRQ;
		uint8_t EnableIRQ;
		bool MasterEnable;
		uint8_t IRQFlags;
		bool MasterFlag;
	}InterruptFields_t;

	typedef struct
	{
		uint32_t D_MADR; //DMA Memory Address Register - Pointer to the virtual address the DMA will start reading from/writing to.
		uint32_t D_BCR; //DMA Block Control Register

		/*
			0       Transfer Direction    (0=To Main RAM, 1=From Main RAM)
			1       Memory Address Step   (0=Forward;+4, 1=Backward;-4)
			2-7     Not used              (always zero)
			8       Chopping Enable       (0=Normal, 1=Chopping; run CPU during DMA gaps)
			9-10    SyncMode, Transfer Synchronisation/Mode (0-3):
				0  Start immediately and transfer all at once (used for CDROM, OTC)
				1  Sync blocks to DMA requests   (used for MDEC, SPU, and GPU-data)
				2  Linked-List mode              (used for GPU-command-lists)
				3  Reserved                      (not used)
			11-15   Not used              (always zero)
			16-18   Chopping DMA Window Size (1 SHL N words)
			19      Not used              (always zero)
			20-22   Chopping CPU Window Size (1 SHL N clks)
			23      Not used              (always zero)
			24      Start/Busy            (0=Stopped/Completed, 1=Start/Enable/Busy)
			25-27   Not used              (always zero)
			28      Start/Trigger         (0=Normal, 1=Manual Start; use for SyncMode=0)
			29      Unknown (R/W) Pause?  (0=No, 1=Pause?)     (For SyncMode=0 only?)
			30      Unknown (R/W)
			31      Not used              (always zero)
		*/
		uint32_t D_CHCR; //DMA Channel Control Register
	}ChannelRegisters_t;

	enum ChannelControlFields
	{
		TransferDirection = 0x1,
		MemoryAddressStep = 0x2,
		ChoppingEnable = 0x100,
		SyncMode = 0x600,
		ChoppingDMAWindowSize = 0x70000,
		ChoppingCPUWindowSize = 0x700000,
		StartBusy = 0x1000000,
		StartTrigger = 0x10000000
	};

	typedef struct
	{
		uint8_t TransferDirection : 1; //0 - to main ram; 1 - from main ram
		uint8_t MemoryAddressStep : 1; //0 - forward (+4 bytes); 1 - backward (-4 bytes)
		uint8_t Unused1 : 6;
		uint8_t ChoppingEnable : 1;
		uint8_t SyncMode : 2; //0 - Start immediately and transfer all at once; 1- Sync blocks to DMA requests; 2 - linked list mode;
		uint8_t Unused2 : 5;
		uint8_t ChoppingDMAWindowSize : 3;
		uint8_t Unused3 : 1;
		uint8_t ChoppingCPUWindowSize : 3;
		uint8_t Unused4 : 1;
		uint8_t StartBusy : 1; // (0=Stopped/Completed, 1=Start/Enable/Busy)
		uint8_t Unused5 : 3;
		uint8_t StartTrigger : 1; //(0=Normal, 1=Manual Start; use for SyncMode=0)
		uint8_t Unused6 : 3;
	}ChannelControl_t;

private:


	uint32_t Control;
	uint32_t Interrupt;
	ChannelRegisters_t Channel[7];
	Memory* pMemory;

	bool DMA0Enabled;
	bool DMA1Enabled;
	bool DMA2Enabled;
	bool DMA3Enabled;
	bool DMA4Enabled;
	bool DMA5Enabled;
	bool DMA6Enabled;

	static InterruptFields_t CreateInterruptFieldFromInt(uint32_t interrupt_field);
	static ChannelControl_t CreateChannelControlFieldFromInt(uint32_t channel_control);
	void SetChannelMADR(uint8_t channel, uint32_t data);
	void SetChannelBCR(uint8_t channel, uint32_t data);
	void SetChannelCHCR(uint8_t channel, uint32_t data);

	uint32_t GetChannelMADR(uint8_t channel) const;
	uint32_t GetChannelBCR(uint8_t channel) const;
	uint32_t GetChannelCHCR(uint8_t channel) const;
	void DoDMA(ChannelRegisters_t& rChannel, uint8_t channelNum);
	void Done(uint8_t ChannelNum);
	uint32_t GetToDeviceAddress(uint8_t ChannelNumber);
public:
	enum DMAControlFields
	{
		MDECInDMA0Priority = 0x07,
		MDECInDMA0Enable = 0x08,
		MDECOutDMA1Priority = 0x70,
		MDECOutDMA1Enable = 0x80,
		GPUDMA2Priority = 0x700,
		GPUDMA2Enable = 0x800,
		CDROMDMA3Priority = 0x7000,
		CDROMDMA3Enable = 0x8000,
		SPUDMA4Priority = 0x70000,
		SPUDMA4Enable = 0x80000,
		PIODMA5Priority = 0x700000,
		PIODMA5Enable = 0x800000,
		OTCDMA6Priority = 0x7000000,
		OTCDMA6Enable = 0x8000000
	};

	enum DMAInterruptFields
	{
		ForceIRQ = 0x8000,
		EnableIRQ = 0x7F0000,
		MasterEnableIRQ = 0x800000,
		IRQFlags = 0x7F000000,
		MasterFlag = 0x80000000
	};

	Dma(Memory* memory);
	virtual ~Dma();
	uint32_t ReadControl() const;
	void WriteControl(uint32_t data);
	uint32_t ReadInterrupt() const;
	void WriteInterrupt(uint32_t data);

	void WriteToDMARegister(uint32_t address, uint32_t data);
	uint32_t ReadFromDMARegister(uint32_t address) const;
};

