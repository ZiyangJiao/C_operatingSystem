#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <asm/unistd.h>
#include <errno.h>


int main(){
	int sys_call_num = 398;
	printf("syscall to getuid returned: %s\n", syscall(sys_call_num));
	printf("getuid returned again: %s\n", syscall(399, 10));
	return 0;
}

