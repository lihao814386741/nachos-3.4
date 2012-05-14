/*
	Just a Hello World Write By LIHAO
*/
#include "syscall.h"


int 
main()
{
	OpenFileId * result;
	char *a;
	Create("a");
	result = Open("a");
	Read(a, 126, result);
	Close(result);
}
