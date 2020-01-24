#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
int main (){
 
 printf("I'm a parent process! My pid is %d!",(int)getpid());

	pid_t pid = fork();
	if ( pid == 0 ) {
		 printf("I'm a child process! My pid is %d! My parent's pid is %d!",(int)getpid(),(int)getppid());
   exit(0);
	}

	wait( 1 );

	printf( "Finished!\n" );

 return 0;
}
