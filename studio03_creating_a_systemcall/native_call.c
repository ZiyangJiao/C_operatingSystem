#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <asm/unistd.h>
#include <errno.h>


int main(){
	printf("syscall to getuid returned: %u\n", syscall(__NR_getuid));
	int return_val = syscall(__NR_setuid,0); 
	printf("syscall to setuid as 0: %d\n", return_val);
	if(return_val !=0) {
		printf("Error: setuid failed! Reason: %s\n", strerror(errno));
	}
	printf("getuid returned again: %u\n", syscall(__NR_getuid));
	return 0;
}

