/*
	Just a Hello World Write By LIHAO
*/
#include "syscall.h"


int 
main()
{
	OpenFileId * result;
	Create("a");
	result = Open("a");
	Write("It's a Test file", 16, result);
	Close(result);
}
