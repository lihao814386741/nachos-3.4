// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "syscall.h"
#include "system.h"
#include "thread.h"
#include "filehdr.h"
#include "filesys.h"
#include "directory.h"
#include "utility.h"
#include "synch.h"
#define TLB_FIFO

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
//EDIT BY LIHAO
int ExitValue = 0;
int TlbMissCount = 0;
int PageFaultCount = 0;
void dummy(int a)
{
   ASSERT(currentThread->space != NULL);
    if (currentThread->space != NULL) { // if there is an address space
             currentThread->RestoreUserState(); // to restore, do it.                
              currentThread->space->RestoreState();
       }
        machine->Run();
 }

int ReadFromFile(OpenFile * file)
{
	int buffer = machine -> ReadRegister(4);
	int size = machine -> ReadRegister(5);
	int result = 0;
	

	if (size > 0 && size <= 128)
	{
		char temp[128];
		result = file -> Read(temp, size);
		for (int i = 0; i < size; ++ i)
		{
			machine -> WriteMem(buffer + i, 1, (int)(temp[i]));
		}
		printf("buffer:%s.\n", temp);
	}

	machine -> WriteRegister(2, result);
		machine -> WriteRegister(PrevPCReg, machine -> ReadRegister(PCReg));
		machine -> WriteRegister(PCReg, machine -> ReadRegister(NextPCReg));
		machine -> WriteRegister(NextPCReg, machine -> ReadRegister(PCReg) + 4);
	return result;
}

int WriteToFile(OpenFile * file)
{
	int buffer = machine -> ReadRegister(4);
	int size = machine -> ReadRegister(5);
	int result = 0;

	if (size > 0 && size <= 128)
	{
		char temp[128];

		for (int i = 0; i < size; ++ i)
		{
			machine -> ReadMem(buffer + i, 1, (int *)(&temp[i]));
		}
		result = file -> Write(temp, size);
	}

	machine -> WriteRegister(2, result);
		machine -> WriteRegister(PrevPCReg, machine -> ReadRegister(PCReg));
		machine -> WriteRegister(PCReg, machine -> ReadRegister(NextPCReg));
		machine -> WriteRegister(NextPCReg, machine -> ReadRegister(PCReg) + 4);
	return result;

}
void WriteToConsole(char* outFile){
	int bufferOffset = machine->ReadRegister(4);
	int bufferNum = machine->ReadRegister(5);

	int ch;
	DEBUG('e', "[%d]Write Begin\n",stats->totalTicks);
	for( int i = 0; i < bufferNum; i++){
		if( machine->ReadMem(bufferOffset + i , 1, (&ch) ) == false )
			return;
		//SynchCmd->PutChar((char)ch);
		DEBUG('e', "[%d]Write char:%c, bufferNum:%d.\n",stats->totalTicks,ch,bufferNum);
		cout<< (char)ch;
	}
	DEBUG('e', "[%d]Write End\n",stats->totalTicks);
	
	
	machine->WriteRegister(2,0 );
	machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg)+4);
}
void ReadFromConsole(char* inFile){
	int bufferOffset = machine->ReadRegister(4);
	int bufferNum = machine->ReadRegister(5);
	char ch;
	for( int i = 0; i < bufferNum; i++){
		//machine->WriteMem(bufferOffset + i , 1, (int)SynchCmd->GetChar() );
		if( scanf("%c",&ch) != EOF )
			if( machine->WriteMem(bufferOffset + i , 1, (int)ch ) == false )
				return;
		else
			break;
	}
	
	machine->WriteRegister(2,0 );
	machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg)+4);
}
//END
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    int result = 0;

	//EDIT BY LIHAO
    if ((which == SyscallException)) {
	    if (type == SC_Halt)
	    {
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
	    }
	    else if (type == SC_Create)
	    {
	    	char fileName[FULL_PATH_LENGTH + 1] = {0};
		int bufOffset = machine -> ReadRegister(4);

		for (int i = 0; i < FULL_PATH_LENGTH; ++ i)
		{
			if (machine -> ReadMem(bufOffset + i, 1, (int *)(&fileName[i])) == false)
			{
				return;
			}
			if (fileName[i] == 0)
				break;
		}
		
		printf("CREATE FILENAME == %s.\n", fileName);
		int result = (int)fileSystem -> Create(fileName, 128);
		printf("result == %d.\n", result);
		DEBUG('a', "[]Create file: %s, result : %d\n", fileName, result);

		machine -> WriteRegister(2, result);
		machine -> WriteRegister(PrevPCReg, machine -> ReadRegister(PCReg));
		machine -> WriteRegister(PCReg, machine -> ReadRegister(NextPCReg));
		machine -> WriteRegister(NextPCReg, machine -> ReadRegister(PCReg) + 4);
	    }
	    else if (type == SC_Open)
	    {
	    	char fileName[FULL_PATH_LENGTH + 1] = {0};
		int bufOffset = machine -> ReadRegister(4);

		for (int i = 0; i < FULL_PATH_LENGTH; ++ i)
		{
			if (machine -> ReadMem(bufOffset + i, 1, (int *)(&fileName[i])) == false)
			{
				return;
			}
			if (fileName[i] == 0)
				break;
		}
		
		printf("OPEN FILENAME == %s.\n", fileName);
		int result = (int)fileSystem -> Open(fileName);
		printf("the result is %d.\n", result);
		
		machine -> WriteRegister(2, result);
		machine -> WriteRegister(PrevPCReg, machine -> ReadRegister(PCReg));
		machine -> WriteRegister(PCReg, machine -> ReadRegister(NextPCReg));
		machine -> WriteRegister(NextPCReg, machine -> ReadRegister(PCReg) + 4);
	    }
	    else if (type == SC_Close)
	    {
	    	int closeFile = machine -> ReadRegister(4);
		if (closeFile != 0)
		{
			printf("Close File Pointer: %d.\n", closeFile);
	//		fileSystem -> Close((OpenFile *)closeFile);
			delete (OpenFile *)closeFile;
		}
	    
	    
	    	machine -> WriteRegister(PrevPCReg, machine -> ReadRegister(PCReg));
		machine -> WriteRegister(PCReg, machine -> ReadRegister(NextPCReg));
		machine -> WriteRegister(NextPCReg, machine -> ReadRegister(PCReg) + 4);
	    }
	    else if (type == SC_Read)
	    {
			int readType = machine->ReadRegister(6);
			if( readType == ConsoleInput ){
				ReadFromConsole(NULL);
			}
			else{
				OpenFile* file = (OpenFile*)readType;
				ReadFromFile( file );
			}


	    }
	    else if (type == SC_Write)
	    {
			int writeType = machine->ReadRegister(6);
			if( writeType == ConsoleOutput ){
				WriteToConsole(NULL);
			}
			else{
				OpenFile* file = (OpenFile*)writeType;
				WriteToFile( file );
			}
	    }
	    else if (type == SC_Exit)
	    {
	    	printf("Enter Exit.\n");
	    	ExitValue = machine -> ReadRegister(4);

		currentThread -> Finish();
	    }
	    else if( type == SC_Exec ){							//2
			DEBUG('j', "Enter Exec\n" );
			char fileName[FULL_PATH_LENGTH + 1]={0};
			int bufOffset = machine->ReadRegister(4);
			for( int i = 0; i < FULL_PATH_LENGTH; i++ ){
				machine->ReadMem( bufOffset + i, 1, (int*)(&fileName[i]) );
				if( fileName[i] == '\0' )
					break;
			}
			OpenFile *executable = fileSystem->Open(fileName);
			
			if( executable != NULL ){
				Thread* newThread = new Thread(fileName);
				AddrSpace *sspace;
				DEBUG('j', "ExecThread: %s\n",fileName );
				
				int UserReg[40];
				char mainMem[MEMORY_SIZE+1];
				
				for (int i = 0; i < 40; i++)
					UserReg[i] = machine->ReadRegister(i);
	//			printf("before:\n");
	//			for (int i = 0; i < MEMORY_SIZE; ++i)
	//			{
	//				printf("%c", machine -> mainMemory[i]);
	//			}
	//			printf("\n");
				memcpy(mainMem,machine->mainMemory,MEMORY_SIZE);
				TranslationEntry *pTable = machine->pageTable;
				int nPages =  machine->pageTableSize;
				sspace = new AddrSpace(executable);
				sspace->InitRegisters();
				newThread->space = sspace;
				newThread->SaveUserState();

				newThread->Fork(dummy,0);
				currentThread -> Yield();
				result = newThread -> getThreadId();
		//		printf("CREATE Tid:%d\n", result);

				delete executable;
				
				for (int i = 0; i < 40; i++)
					machine->WriteRegister(i,UserReg[i]);
				//printf("MEMORY_SIZE:%d\n", MEMORY_SIZE);
				memcpy(machine->mainMemory,mainMem,MEMORY_SIZE);
	//			printf("after:\n");
	//			for (int i = 0; i < MEMORY_SIZE; ++i)
	//			{
	//				printf("%c", machine -> mainMemory[i]);
	//			}
		//		printf("\n");
				machine->pageTable = pTable;
				machine->pageTableSize = nPages;
				
			}
			else{
				printf("Unable to open file %s\n", fileName);
			}
			
			machine->WriteRegister(2,result);
			machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
			machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
			machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg)+4);
			
		}
		else if( type == SC_Join ){						//3
			int tid = machine->ReadRegister(4);
			//printf("SC_JOIN GET TID:%d.\n", tid);
			if(tid != 0 ){
				std::map<int, Thread*>::iterator itFind;
				if( (itFind = ThreadList.find(tid) ) != ThreadList.end() ){
					Thread* thr = (*itFind).second;
					ASSERT(thr !=NULL);
					thr->JoinLock->Acquire();
					thr->JoinNum++;
					
					thr->JoinCond->Wait(thr->JoinLock);
					
					thr->JoinLock->Release();
				}
			}

			machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
			machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
			machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg)+4);
			
		}
	    //END
    }
    else if (which == PageFaultException)
    {


	int badAddr = machine -> ReadRegister(BadVAddrReg);
	int vmIndex = badAddr / PageSize;
	int i;
	TranslationEntry *entry = &(currentThread -> space -> pageTable[vmIndex]);

	if (entry -> valid) 			//Exchange The TLB
	{
		
		TlbMissCount ++;
		DEBUG('V', "TLB MISS PC = [%d]\tTLBMISSCOUNT = [%d]\n", machine -> ReadRegister(PCReg), TlbMissCount);
		for (i = 0; i < TLBSize; ++ i)	
		{
			if (machine -> tlb[i].valid == false)
			{
				machine -> tlb[i].virtualPage = entry -> virtualPage;
				machine -> tlb[i].physicalPage= entry -> physicalPage;
				machine -> tlb[i].valid 	   = TRUE;
				machine -> tlb[i].readOnly    = entry -> readOnly;
				machine -> tlb[i].use	   = entry -> use;
				machine -> tlb[i].dirty  	   = entry -> dirty;
				break;
			}
		}
		if (i == TLBSize)
		{
			int pro = 0;
			int exchange = -1;
#ifdef TLB_NRU
			for (i = 0; i < TLBSize; ++ i)
			{
				if (machine -> tlb[i].use == false && machine -> tlb[i].dirty == false)
				{
					exchange = i;
					pro = 4;
					break;
				}
				else if (machine -> tlb[i].use == false && machine -> tlb[i].dirty == true && pro < 3)
				{
					exchange = i;
					pro = 3;
				}
				else if (machine -> tlb[i].use == true && machine -> tlb[i].dirty == false && pro < 2)
				{
					exchange = i;
					pro = 2;
				}
				else if (machine -> tlb[i].use == true && machine -> tlb[i].dirty == true && pro < 1)
				{
					exchange = i;
					pro = 1;
				}
			}
#endif
#ifdef TLB_FIFO
			
			int min = -1;
			for (i = 0; i < TLBSize; ++ i)
			{
				if (min == -1 || min < machine -> tlb[i].intime)
				{
					exchange = i;
				}
			}


#endif


			for (i = 0; i < machine -> pageTableSize; ++ i)
			{
				if (machine -> pageTable[i].physicalPage == machine -> tlb[exchange].physicalPage && machine -> pageTable[i].valid )
				{
					machine -> pageTable[i].virtualPage = machine -> tlb[exchange].virtualPage;
					machine -> pageTable[i].readOnly = machine -> tlb[exchange].readOnly;
					machine -> pageTable[i].use = machine -> tlb[exchange].use;	
					machine -> pageTable[i].dirty = machine -> tlb[exchange].dirty ;
					machine -> pageTable[i].intime = stats -> totalTicks;
					break;
				}
			}




				machine -> tlb[exchange].virtualPage = entry -> virtualPage;
				machine -> tlb[exchange].physicalPage= entry -> physicalPage;
				machine -> tlb[exchange].valid 	   = TRUE;
				machine -> tlb[exchange].readOnly    = entry -> readOnly;
				machine -> tlb[exchange].use	   = entry -> use;
				machine -> tlb[exchange].dirty  	   = entry -> dirty;
				machine -> tlb[exchange].intime = stats -> totalTicks; 
		
		}
	}
	else
	{
		
		PageFaultCount ++;
		DEBUG('V', "Page Fault. Addr = [%d]\tCount = [%d]\tThe PC = [%d]\n", badAddr, PageFaultCount, machine -> ReadRegister(PCReg));
		char MemorySectorTemp[128];
		fileSystem -> SwapFile -> ReadAt(MemorySectorTemp, 128, (entry -> virtualPage) * 128);

		if (physicalMemoryMap -> NumClear() > 0)//Have Enough Physical Memory
		{
			entry -> valid = true;
			entry -> physicalPage = physicalMemoryMap -> Find();
			entry -> intime = stats -> totalTicks;
			memcpy(&(machine -> mainMemory[entry -> physicalPage * 128]), MemorySectorTemp, 128);
			DEBUG('V', "PAGE FAULT, NEW EMPTY MEMORY, STORED IN PM:[%d]\n", entry -> physicalPage);
			int pc = machine -> ReadRegister(NextPCReg);
			DEBUG('V', "THE PC IS :[%d]\n", pc);
		}
		else 
		{
			int exchange = -1;
			int minTime = 0;
			
			for (int i = 0; i < machine -> pageTableSize; ++ i)
			{
				if (minTime > machine ->pageTable[i].intime || minTime == 0)
				{
					minTime = machine ->pageTable[i].intime;
					exchange = i;
				}
			}
			
			for (int i = 0; i < TLBSize; ++ i)
			{
				if (machine -> tlb[i].valid && machine -> tlb[i].physicalPage == machine ->pageTable[exchange].physicalPage);
				{
					machine -> tlb[i].valid = false;
				}
			}

		
			if (machine ->pageTable[i].dirty)
			{
				fileSystem -> SwapFile -> WriteAt(&(machine -> mainMemory[machine ->pageTable[exchange].physicalPage * 128]), 128, machine ->pageTable[exchange].virtualPage * 128);	
			}
			machine ->pageTable[exchange].valid = false;
			machine ->pageTable[exchange].physicalPage = -1;
			physicalMemoryMap -> Clear(machine -> pageTable[exchange].physicalPage);
		
		}
	
	}

		
	
    }

    else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
