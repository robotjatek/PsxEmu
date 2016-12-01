#include <iostream>
#include "Memory.h"
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
 -----------------------------------------------------------------------------------------------------
 SIGN EXTENSION: ha az unsigned �rt�ket el�tt egy ugyanolyan m�ret� signedra kasztolom, majd berakom egy nagyobb m�ret� t�rol�ba, akkor a sign extension automatikusan v�gbemegy
 nagyobb m�ret� unsignedb�l kissebb m�ret� signedba �trakva is j� a sign
 -----------------------------------------------------------------------------------------------------
 Branch and link mindig elmenti a c�met az RA regiszterbe, akkor is, ha a felt�tel nem teljes�l
 --------------------------------------------------------------------------------------------------------
 DIV m�velet: 36 ciklus, a pipeline nem �ll le, az utas�t�sokat folyamatosan v�grehajtja tov�bbra is, majd 36 ciklus ut�n az eredm�ny a hi �s lo regiszterekben
 ---------------------------------------------------------------------------------------------------------------------------------------

 IRQ jegyzetek:

 IRQ ack szoftveresen van megval�s�tva: Az I_stat regiszter (0x1F801070) megfelel� bitj�t az exception handler kinull�zza fut�s ut�n
 Az I_stat regiszter megfelel� bitj�t be kell billenteni IRQ request est�n
 Ha a bit az I_mask regiszerben maszkolva van, akkor az interrupt nem fut le

 K�rd�s: A hardver mit csin�l miut�n az irq flag be lett �ll�tva?
 Val�sz�n�leg csak sim�n megh�vja az exception handlert IRQ param�terrel


 --------------------------------------------------------------
 DMA: egy csom� m�dot t�mogat a hardver, de �gy n�z ki, a j�t�kok csak n�h�nyat haszn�lnak

-----------------------------------------------------------------

https://www.youtube.com/watch?v=zBkNBP00wJE&feature=youtu.be&t=26m55s egy nagyon tanuls�gos videor�szlet a const haszn�lat�r�l 26:55-28:10
*/



/*
TODO: a pipelinet lehet hogy emul�lni k�ne
*/


//TODO: a decodeban az indirekt f�ggv�nyh�v�sokat switch-re cser�lni

//TODO: Threads of Fate c. jatek division by zero "famous emulation error"
//TODO: formula one 2001 r3000 do not update the I-cache on write to main memory
//TODO: By convention, if an exception or interrupt prevents the completion of an instruction in the branch delay slot, the instruction stream is continued by re - executing the branch instruction.

int main()
{
	Memory memory;
	memory.RunSystem();
	
	system("pause");
	return 0;
}
