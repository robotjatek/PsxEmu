#include <iostream>
#include "Memory.h"
#include "R3000A.h"
//#include <vld.h>

/*
NOTES

--------------------------------------------------
MIPS
Microprocessor without Interlocked Pipeline Stages
--------------------------------------------------

--------------------------------------------------
DELAY SLOT
the instruction after a branch, a jump or a load is always executed! Even if the branch or jump were taken!
 + a load után következõ utasítás még nem rendelkezik a load értékével!
 --------------------------------------------------

 Threads of Fate c. jatek division by zero "famous emulation error"
 formula one 2001 r3000 do not update the I-cache on write to main memory
 -----------------------------------------------------------------------------
 scratcpad: egymásnak ellentmondó adatok: 1k vagy 4k méretek...

 -----------------------------------------------------------------------------------------------------
 van külön a bios a BIOS_START címen, és van a kernel memory space a 0x0 fizikai címtõl

*/



/*
TODO: a pipelinet lehet hogy emulálni kéne
*/

//TODO: sign extension - negatív offsettel tesztelni az elágazásokat

//TODO: a decodeban az indirekt függvényhívásokat switch-re cserélni

//TODO: Threads of Fate c. jatek division by zero "famous emulation error"
//TODO: formula one 2001 r3000 do not update the I-cache on write to main memory
//TODO: By convention, if an exception or interrupt prevents the completion of an instruction in the branch delay slot, the instruction stream is continued by re - executing the branch instruction.

int main()
{
	Memory memory;
	memory.load_binary_to_bios_area("asd.txt");
	R3000A r3000(memory);
	r3000.Run();
	system("pause");
	return 0;
}