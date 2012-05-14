//EDIT BY LIHAO
#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H



#include "copyright.h"
#include "synch.h"
#include "console.h"


extern void MySynchReadAvail(int arg);
extern void MySynchWriteDone(int arg);

class SynchConsole{
	public:
		SynchConsole(char * inputFile = NULL, char * outputFile = NULL);
		~SynchConsole();

		char GetChar();
		void PutChar(char c);
		void ReadAvail();
		void WriteDone();

	private:
		Console *console;
		Lock *lock;
		Semaphore * SynchReadAvail;
		Semaphore * SynchWriteDone;
};



#endif //SYNCHCONSOLE_H
//END

