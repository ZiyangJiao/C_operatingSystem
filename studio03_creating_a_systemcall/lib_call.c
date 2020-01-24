#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>


int main(){
	printf ("getuid returned: %u\n", getuid());
	int return_val = setuid(0);
	printf("%d\n", return_val);
	if(return_val != 0) {
		printf("Error: setuid failed! Reason: %s\n", strerror(errno));
	}
	printf("getuid returned again: %u\n", getuid());
	return 0;
}

