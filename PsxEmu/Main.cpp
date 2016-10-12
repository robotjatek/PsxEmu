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


*/


/*
a pipelinet lehet hogy emul�lni k�ne
*/

//TODO: sign extension - negat�v offsettel tesztelni az el�gaz�sokat

//TODO: a decodeban az indirekt f�ggv�nyh�v�sokat switch-re cser�lni

//XXX: Threads of Fate c. jatek division by zero "famous emulation error"
//XXX: formula one 2001 r3000 do not update the I-cache on write to main memory

int main()
{
	Memory memory;
	memory.load_binary_to_bios_area("asd.txt");
	R3000A r3000(memory);
	r3000.Step();
	system("pause");
	return 0;
}