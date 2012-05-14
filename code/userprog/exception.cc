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
#include "system.h"
#include "syscall.h"
#include "thread.h"
#include "system.h"
#include "filehdr.h"
#include "filesys.h"
#include "directory.h"
#include "synch.h"
#include "utility.h"

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
int ReadFromFile(OpenFile * file)
{
	int buffer = machine -> ReadRegister(4);
	int size = machine -> ReadRegister(5);
	int result = 0;
	

	if (size > 0 && size <= 128)
	{
		char temp[size];
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
//END
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

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
	    	int read = machine -> ReadRegister(6);

		OpenFile *file = (OpenFile *)read;
		printf("Read FILE POINTER:%d.\n", read);
		ReadFromFile(file);
	    }
	    else if (type == SC_Write)
	    {
	    	int writeType = machine -> ReadRegister(6);
	    
	    	OpenFile * file = (OpenFile *)writeType;
		
	    
	    }
	    //END
    }

    else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
