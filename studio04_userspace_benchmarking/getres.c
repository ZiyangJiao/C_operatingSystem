#include<stdio.h>
#include<time.h>

int main (){
 struct timespec realtime;
 struct timespec realtime_coarse;
 struct timespec monotonic;
 struct timespec monotonic_coarse;
 struct timespec monotonic_raw;
 struct timespec boottime;
 struct timespec process_cputime;
 struct timespec thread_cputime;

 clock_getres(CLOCK_REALTIME, &realtime);
 clock_getres(CLOCK_REALTIME_COARSE, &realtime_coarse);
 clock_getres(CLOCK_MONOTONIC, &monotonic);
 clock_getres(CLOCK_MONOTONIC_COARSE, &monotonic_coarse);
 clock_getres(CLOCK_MONOTONIC_RAW, &monotonic_raw);
 clock_getres(CLOCK_BOOTTIME, &boottime);
 clock_getres(CLOCK_PROCESS_CPUTIME_ID, &process_cputime);
 clock_getres(CLOCK_THREAD_CPUTIME_ID, &thread_cputime);
 printf("realtime resolution: tv_sec:%ld tv_nsec:%ld\n",realtime.tv_sec,realtime.tv_nsec);
 printf("realtime_coarse resolution: tv_sec:%ld tv_nsec:%ld\n",realtime_coarse.tv_sec,realtime_coarse.tv_nsec);
 printf("monotonic resolution: tv_sec:%ld tv_nsec:%ld\n",monotonic.tv_sec,monotonic.tv_nsec);
 printf("monotonic_coarse resolution: tv_sec:%ld tv_nsec:%ld\n",monotonic_coarse.tv_sec,monotonic_coarse.tv_nsec);
 printf("monotonic_raw resolution: tv_sec:%ld tv_nsec:%ld\n",monotonic_raw.tv_sec,monotonic_raw.tv_nsec);
 printf("boottime resolution: tv_sec:%ld tv_nsec:%ld\n",boottime.tv_sec,boottime.tv_nsec);
 printf("process_cputime resolution: tv_sec:%ld tv_nsec:%ld\n",process_cputime.tv_sec,process_cputime.tv_nsec);
 printf("thread_cputime resolution: tv_sec:%ld tv_nsec:%ld\n",thread_cputime.tv_sec,thread_cputime.tv_nsec);

return 0;
}
