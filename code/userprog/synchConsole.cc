//EDIT BY LIHAO
#include "copyright.h"
#include "synchConsole.h"
#include "machine.h"
#include "system.h"


void MySynchReadAvail(int arg)
{
	((SynchConsole *)arg) -> ReadAvail();
}
void MySynchWriteDone(int arg)
{
	((SynchConsole *)arg) -> WriteDone();
}
SynchConsole::SynchConsole(char *inputFile, char * outputFile)
{
	console  = new Console(inputFile, outputFile, MySynchReadAvail, MySynchWriteDone, (int)this);	
	lock = new Lock("synchConsole");
	SynchReadAvail = new Semaphore("consoleInput", 0);
	SynchWriteDone = new Semaphore("consoleOutput", 0);
}
SynchConsole::~SynchConsole()
{
	delete	console;	
	delete	lock; 
	delete	SynchReadAvail;
	delete	SynchWriteDone;
}
char 
SynchConsole::GetChar()
{
	char ch;

	lock -> Acquire();
	SynchReadAvail -> P();
	ch = console -> GetChar();
	lock -> Release();

	return ch;
}
void  
SynchConsole::PutChar(char ch)
{
	lock -> Acquire();
	console -> PutChar(ch);
	SynchWriteDone -> P();
	lock -> Release();
}
void 
SynchConsole::ReadAvail()
{
	SynchReadAvail -> V();
}
void
SynchConsole::WriteDone()
{
	SynchWriteDone -> V();
}
//END
