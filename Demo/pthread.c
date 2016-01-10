#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NTHREADS 2

void *myFun(void *x)
{
//	sleep(2);
	
  int tid,i;
	for(i=0;i<6;i++){
  tid = *((int *) x);
  printf("Hi from thread %d!\n", tid);
	sleep(1);
}
//	sleep(2);
  return NULL;
}

int main()
{
  pthread_t threads[NTHREADS];
  int thread_args[NTHREADS];
  int rc, i,j;


 /* spawn the threads */
//  for (i=0; i<2; i++)
  //  {
      thread_args[i] = i;
      printf("spawning thread %d\n", i);
      rc = pthread_create(&threads[i], NULL, myFun, (void *) &thread_args[i]);
	
	for(j=0;j<9;j++){
	printf("Hai%d\n",j);
	
	sleep(2);
	}
   // }
//	int a;
//	scanf("%d",&a);
  /* wait for threads to finish */
//  for (i=0; i<9; ++i) {
  //  rc = pthread_join(threads[i], NULL);
 // }

  return 1;
}
