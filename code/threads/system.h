// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "synch.h"
#include "iostream"
#include "map"
#include "vector"

#define MAX_THREAD 128
#define MAX_MAIL_NUM 32
#define MAX_MAIL_LENGTH 32
using namespace std;

// Initialization and cleanup routines
struct MailMsg{
	Thread* sendThread;
	Thread* recvThread;
	char data[MAX_MAIL_LENGTH];
};
extern std::vector<MailMsg*> ThreadMailBox;
extern Semaphore* MailLock;
extern int MailCount;
extern int PostMail(Thread * send, Thread * recv, char * data, int len);
extern int GetMail(Thread *send, Thread *recv, char *data);


extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.


extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

extern bool thread_id_is_used[MAX_THREAD + 1];
extern char *data[MAX_THREAD];
extern void freeThreadId(int thread_id);
extern int findFreeThreadId();
#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
