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
 + a load ut�n k�vetkez� utas�t�s m�g nem rendelkezik a load �rt�k�vel!
 --------------------------------------------------

 Threads of Fate c. jatek division by zero "famous emulation error"
 formula one 2001 r3000 do not update the I-cache on write to main memory
 -----------------------------------------------------------------------------
 scratcpad: egym�snak ellentmond� adatok: 1k vagy 4k m�retek...

 -----------------------------------------------------------------------------------------------------
 van k�l�n a bios a BIOS_START c�men, �s van a kernel memory space a 0x0 fizikai c�mt�l

*/



/*
TODO: a pipelinet lehet hogy emul�lni k�ne
*/

//TODO: sign extension - negat�v offsettel tesztelni az el�gaz�sokat

//TODO: a decodeban az indirekt f�ggv�nyh�v�sokat switch-re cser�lni

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