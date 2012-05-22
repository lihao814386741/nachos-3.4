/* Just a Hello World Write By LIHAO
*/
#include "syscall.h"


int 
main()
{
	OpenFileId * result;
	char a[120];
	Create("a");
	result = Open("a");
	Read(a, 119, result);
	Close(result);
}
