// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

// testnum is set in main.cc
//
#include "system.h"
#include "copyright.h"
#include "threadtest.h"
#include "synch.h"
//----------------------------------------------------------------------
//param used in producer and consumer
//
//
//
int testnum = 1;
int con_number = 0;
Lock* mutex = new Lock("mutex");
Lock* producer_lock = new Lock("prol");
Lock* consumer_lock = new Lock("conl");
Condition* producer_condition = new Condition("proc");
Condition* consumer_condition = new Condition("conc");
//----------------------------------------------------------------------
//params used in send and receiver
//
//
//
//
int sender_id = -1;





//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", currentThread -> getThreadId(), num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//----------------------------------------------------------------------
// ThreadTestMulti
// 	Set up a ping-pong between Multi threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------
//

void
ThreadTestMulti(int testnum)
{
    DEBUG('t', "Entering ThreadTestMulti");

    for (int i = 0; i < testnum; ++ i)
    {
	    Thread *t = new Thread("forked thread" );

	    t->Fork(SimpleThread, i);
    }

}



//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    default:
	ThreadTestMulti(testnum);
//	printf("No test specified.\n");
	break;
    }
}

void producer(int a)
{
	for (int i = 1; i < 16; ++ i)	
	{
		mutex -> Acquire();	

		while (con_number != 0)
		{
			producer_lock -> Acquire();
			producer_condition -> Wait(producer_lock);
			producer_lock -> Release();
		}

		con_number = i;
		printf("Producer %d produced %d!!\n", currentThread -> getThreadId(), i);

		consumer_lock -> Acquire();
		consumer_condition -> Signal(consumer_lock);
		consumer_lock -> Release();
		mutex -> Release();

		currentThread -> Yield();
	}

}
void consumer(int a)
{
	int i;
	for (int i = 1; i < 16; ++ i)
	{
		mutex -> Acquire();
		while (con_number == 0)
		{
			consumer_lock -> Acquire();
			consumer_condition -> Wait(consumer_lock);
			consumer_lock -> Release();
		}

		con_number = 0;
		printf("Consumer %d consumed %d!\n", currentThread -> getThreadId(), i);

		producer_lock -> Acquire();
		producer_condition-> Signal(producer_lock);	
		producer_lock -> Release();
		mutex -> Release();
		currentThread -> Yield();
	}
}
void ForkTest()
{
	Thread *t1 = new Thread("producer");
	Thread *t2 = new Thread("consumer");


	t1 -> Fork(producer, 0);
	t2 -> Fork(consumer, 0);
}



void PriorityTest()
{
	int priority[10] = {1, 2, 3, 4, 5, 6, 7, 8, 10, 10};
	Thread* t[10];

	for (int i = 0; i < 10; ++ i)
	{

		char thread_name[128];
		sprintf(thread_name, "%d", i);
		t[i] = new Thread(thread_name);

		t[i] -> setPriority(priority[i]);

		t[i] -> Fork(SimpleThread, t[i] -> getThreadId());

	}

}

void sender(int a)
{
	char *hello = "hello world";
	currentThread -> Send(hello);
	sender_id = currentThread -> getThreadId();
	printf("Send Data:%s\n", hello);

}
void recver(int a)
{
	char *recv_data;
	if (sender_id != -1)
	{
		recv_data = currentThread -> Receive(sender_id);
		printf("Receiver Data:%s\n", recv_data);
	}
}
void SendRecvTest()
{

	Thread *t1 = new Thread("producer");
	Thread *t2 = new Thread("consumer");


	t1 -> Fork(sender, 0);
	t2 -> Fork(recver, 0);

}







