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
 + a load után következõ utasítás még nem rendelkezik a load értékével!
 --------------------------------------------------

 Threads of Fate c. jatek division by zero "famous emulation error"
 formula one 2001 r3000 do not update the I-cache on write to main memory
 -----------------------------------------------------------------------------
 scratcpad: egymásnak ellentmondó adatok: 1k vagy 4k méretek...

 -----------------------------------------------------------------------------------------------------
 van külön a bios a BIOS_START címen, és van a kernel memory space a 0x0 fizikai címtõl
 -----------------------------------------------------------------------------------------------------
 SIGN EXTENSION: ha az unsigned értéket elõtt egy ugyanolyan méretû signedra kasztolom, majd berakom egy nagyobb méretû tárolóba, akkor a sign extension automatikusan végbemegy
 nagyobb méretû unsignedból kissebb méretû signedba átrakva is jó a sign
 -----------------------------------------------------------------------------------------------------
 Branch and link mindig elmenti a címet az RA regiszterbe, akkor is, ha a feltétel nem teljesül
 --------------------------------------------------------------------------------------------------------
 DIV mûvelet: 36 ciklus, a pipeline nem áll le, az utasításokat folyamatosan végrehajtja továbbra is, majd 36 ciklus után az eredmény a hi és lo regiszterekben
 ---------------------------------------------------------------------------------------------------------------------------------------

 IRQ jegyzetek:

 IRQ ack szoftveresen van megvalósítva: Az I_stat regiszter (0x1F801070) megfelelõ bitjét az exception handler kinullázza futás után
 Az I_stat regiszter megfelelõ bitjét be kell billenteni IRQ request estén
 Ha a bit az I_mask regiszerben maszkolva van, akkor az interrupt nem fut le

 Kérdés: A hardver mit csinál miután az irq flag be lett állítva?
 Valószínûleg csak simán meghívja az exception handlert IRQ paraméterrel


 --------------------------------------------------------------
 DMA: egy csomó módot támogat a hardver, de úgy néz ki, a játékok csak néhányat használnak

-----------------------------------------------------------------

https://www.youtube.com/watch?v=zBkNBP00wJE&feature=youtu.be&t=26m55s egy nagyon tanulságos videorészlet a const használatáról 26:55-28:10
*/



/*
TODO: a pipelinet lehet hogy emulálni kéne
*/


//TODO: a decodeban az indirekt függvényhívásokat switch-re cserélni

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
