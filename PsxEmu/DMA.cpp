#include "DMA.h"
#include "Memory.h"

#include <cstdio>
#include <fstream>

Dma::InterruptFields_t Dma::CreateInterruptFieldFromInt(uint32_t interrupt_field)
{
	InterruptFields_t f;
	f.ForceIRQ = (interrupt_field & DMAInterruptFields::ForceIRQ ? true : false);
	f.EnableIRQ = (uint8_t)((interrupt_field & DMAInterruptFields::EnableIRQ) >> 16);
	f.MasterEnable = (interrupt_field & DMAInterruptFields::MasterEnableIRQ ? true : false);
	f.IRQFlags = (interrupt_field & DMAInterruptFields::IRQFlags) >> 24;
	f.MasterFlag = interrupt_field & DMAInterruptFields::MasterFlag ? true : false;

	return f;
}

Dma::ChannelControl_t Dma::CreateChannelControlFieldFromInt(uint32_t channel_control)
{
	ChannelControl_t chc;
	chc.TransferDirection = channel_control & 0x1;
	chc.MemoryAddressStep = (channel_control & 0x2) >> 1;
	chc.ChoppingEnable = (channel_control & 0x100) >> 8;
	chc.SyncMode = (channel_control & 0x600) >> 9;
	chc.ChoppingDMAWindowSize = (channel_control & 0x70000) >> 16;
	chc.ChoppingCPUWindowSize = (channel_control & 0x700000) >> 20;
	chc.StartBusy = (channel_control & 0x1000000) >> 24;
	chc.StartTrigger = (channel_control & 0x10000000) >> 28;
	chc.Unused1 = 0;
	chc.Unused2 = 0;
	chc.Unused3 = 0;
	chc.Unused4 = 0;
	chc.Unused5 = 0;
	chc.Unused6 = 0;
	return chc;
}

void Dma::SetChannelMADR(uint8_t channel, uint32_t data)
{
	Channel[channel].D_MADR = data; //24-31 bits are always zero
}

void Dma::SetChannelBCR(uint8_t channel, uint32_t data)
{
	Channel[channel].D_BCR = data;
}

void Dma::SetChannelCHCR(uint8_t channel, uint32_t data)
{
	Channel[channel].D_CHCR = data;
	DoDMA(Channel[channel], channel);
}

uint32_t Dma::GetChannelMADR(uint8_t channel)
{
	return Channel[channel].D_MADR;
}

uint32_t Dma::GetChannelBCR(uint8_t channel)
{
	return Channel[channel].D_BCR;
}

uint32_t Dma::GetChannelCHCR(uint8_t channel)
{
	return Channel[channel].D_CHCR;
}

void Dma::DoDMA(ChannelRegisters_t& rChannel, uint8_t channelNum)
{
	ChannelControl_t chc = CreateChannelControlFieldFromInt(rChannel.D_CHCR);
	if (chc.SyncMode == 0 && chc.StartBusy == 1 && chc.StartTrigger == 1)
	{
		log << "Syncmode 0, Channel 6:\n";
		if (channelNum == 6) //Clear Reverse Ordering Table
		{
			printf("Manual DMA mode started on channel %d\n",channelNum);
			uint32_t BC = rChannel.D_BCR & 0xffff; //only the low 16 bits are used in SyncMode 0
			uint32_t address = rChannel.D_MADR & 0xffffff; //the low 24 bits are used as an address
			uint8_t num = (rChannel.D_MADR) >> 24; //number of elements in the ordering table. Should be 0 as this is the initialization
			//int8_t step = chc.MemoryAddressStep ? -4 : 4;
			while (BC > 1)
			{
				log <<BC<<" "<< std::hex << address << " " << std::hex << address - 4 << "\n";
//				pMemory->write_word(address, address - 4); //+4?
				pMemory->Write<uint32_t>(address, address - 4);
				address -= 4;
				BC--;
			}
//			pMemory->write_word(address, 0x00ffffff); //last address in the OTC
			pMemory->Write<uint32_t>(address, 0x00ffffff); //last address in the OTC
			log << std::hex << address << " " << std::hex << 0x00ffffff << "\n";
			log.flush();
			Done(channelNum);
		}
		else
		{
			printf("unimplemented DMA channel (%d) for mode %d\n",channelNum,chc.SyncMode);
		}
	}
	else if (chc.SyncMode == 2 && chc.StartBusy == 1) //only GPU
	{
		pMemory->GetCpu()->StartLogging();
		printf("Syncmode 2 on channel %d started\n", channelNum);
		log << "Syncmode 2 channel " << channelNum << "\n";
		if (chc.TransferDirection == 1)//from main ram
		{
			uint32_t AddressOfNext;
			do
			{
				//uint32_t Header = pMemory->read_word(rChannel.D_MADR);
				uint32_t Header = pMemory->Read<uint32_t>(rChannel.D_MADR);
				uint8_t NumOfPackets = Header >> 24;
				AddressOfNext = Header & 0xffffff;
				for (int i = 1; i <= NumOfPackets; i++)
				{
					//uint32_t ToSend = pMemory->read_word(rChannel.D_MADR + i*4);
					uint32_t ToSend = pMemory->Read<uint32_t>(rChannel.D_MADR + i * 4);
					log <<Header<<" "<< std::hex << rChannel.D_MADR + i * 4 << " Next: " << std::hex << AddressOfNext <<" "<<ToSend<< "\n";
//					pMemory->write_word(0x1f801810, ToSend); //set to gp0. Im not entirely sure if its the right way to do DMA2, Nocash says: "DMA2 is equivalent to accessing Port 1F801810h (GP0/GPUREAD) by software." so i think its ok
					pMemory->Write<uint32_t>(0x1f801810, ToSend); //set to gp0. Im not entirely sure if its the right way to do DMA2, Nocash says: "DMA2 is equivalent to accessing Port 1F801810h (GP0/GPUREAD) by software." so i think its ok
				}
				rChannel.D_MADR = AddressOfNext;
			} while (AddressOfNext != 0xffffff);
			Done(channelNum);
			log.flush();
		}
		else //to main ram
		{
			printf("To main ram direction is not supported\n");
		}
	}
	else if (chc.SyncMode == 1)
	{
		//uint32_t address = rChannel.D_MADR;
		uint32_t BA = rChannel.D_BCR >> 16;
		uint32_t BS = rChannel.D_BCR & 0xffff;
//		int8_t step = chc.MemoryAddressStep ? -4 : 4;
		if (chc.TransferDirection == 1)//from main ram
		{
			for (uint32_t i = 0; i < BA; i++)
			{
				for (uint32_t j = 0; j < BS; j++)
				{
					//uint32_t data = pMemory->read_word(rChannel.D_MADR);
					uint32_t data = pMemory->Read<uint32_t>(rChannel.D_MADR);
//					pMemory->write_word(GetToDeviceAddress(channelNum), data);
					pMemory->Write<uint32_t>(GetToDeviceAddress(channelNum), data);
					rChannel.D_MADR += 4;
				}
			}

			Done(channelNum);
		}
		else
		{
			printf("To main ram direction is not supported\n");
		}
	}
	else
	{
		printf("Unhandled DMA Sync mode!!! %d on channel: %d\n", chc.SyncMode, channelNum);
	}
}

void Dma::Done(uint8_t ChannelNum)
{
	//Disable busy and start bits at the end of the DMA
	Channel[ChannelNum].D_CHCR &= ~ChannelControlFields::StartBusy;
	Channel[ChannelNum].D_CHCR &= ~ChannelControlFields::StartTrigger;

	//IRQ
/*	InterruptFields_t irq = CreateInterruptFieldFromInt(Interrupt);
	if (irq.ForceIRQ == true || (irq.MasterEnable == true && irq.EnableIRQ))
	{
		if ((irq.EnableIRQ & (1 << ChannelNum)))
		{
			Interrupt |= DMAInterruptFields::MasterFlag; //???
			uint32_t mask = DMAInterruptFields::IRQFlags & (1 << ChannelNum);
			Interrupt |= mask; //sets the interrupt flag for the current channel
		//	pMemory->SetIStatFields(Memory::IRQStatFields::DMA);
			//TODO: re-enable interrupts on DMA Done
		}
	}
	else
	{
		Interrupt &= ~DMAInterruptFields::MasterFlag;
	}
	*/
}

uint32_t Dma::GetToDeviceAddress(uint8_t ChannelNumber)
{
	switch (ChannelNumber)
	{
	case 2:
		return 0x1F801810;
		break;
	default:
		printf("Unsupported channel!\n");
		return 0;
		break;
	}
}

Dma::Dma(Memory* memory)
{
	Control = 0x07654321; //From no$psx specification
	pMemory = memory;
	log.open("log.txt", std::ios::out);
}

Dma::~Dma()
{
	log.close();
}

uint32_t Dma::ReadControl()
{
	return Control;
}

void Dma::WriteControl(uint32_t data)
{
	Control = data;
}

uint32_t Dma::ReadInterrupt()
{
	return Interrupt;
}

void Dma::WriteInterrupt(uint32_t data)
{
	data &= ~0x7fc0; //bits 6-14 are always zero
	//irq bits 24-30 are acknowledged (reset to zero) when writing a "1" to that bits
	uint32_t ack = data&DMAInterruptFields::IRQFlags;
	data &= ~ack;
	//TODO: check if this is correct, or i should check for '1' writes individually for all Flag bits
	Interrupt = data;
}

void Dma::WriteToDMARegister(uint32_t address, uint32_t data)
{
	if (address == 0x1f8010f0)
	{
		WriteControl(data);
	}
	else if (address == 0x1f8010f4)
	{
		WriteInterrupt(data);
	}
	else
	{
		uint8_t ChannelNum = ((address >> 4) & 0x7);
		uint8_t ChRegister = address & 0xf;
		switch (ChRegister)
		{
		case 0:
			SetChannelMADR(ChannelNum, data);
			break;
		case 4:
			SetChannelBCR(ChannelNum, data);
			break;
		case 8:
			SetChannelCHCR(ChannelNum, data);
			break;
		default:
			printf("Unhandled DMA register access");
			break;
		}
	}
}

uint32_t Dma::ReadFromDMARegister(uint32_t address)
{
	if (address == 0x1f8010f0)
	{
		return ReadControl();
	}
	else if (address == 0x1f8010f4)
	{
		return ReadInterrupt();
	}
	else
	{
		uint8_t ChannelNum = ((address >> 4) & 0x7);
		uint8_t ChRegister = address & 0xf;

		switch (ChRegister)
		{
		case 0:
			return GetChannelMADR(ChannelNum);
			break;
		case 4:
			return GetChannelBCR(ChannelNum);
			break;
		case 8:
			return GetChannelCHCR(ChannelNum);
			break;
		default:
			printf("Unhandled DMA register access");
			break;
		}
	}

	return 0;
}
