#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>  // uintptr_t
#include <inttypes.h> // PRIdPTR formatting macro

#define NUM_THREADS 10

void *task_hello(void *data)
{
	uintptr_t i = (uintptr_t)data;
	printf("                                        Hello world, data: %p, %" PRIdPTR " \n", data, i);
	pthread_exit((void*)(2*i));
}

int main (int argc, char *argv[])
{
	pthread_t threads[NUM_THREADS];
	int ret;
	uintptr_t i;
	for (i = 0; i < NUM_THREADS; ++i) {
		printf("main: pthread_create %" PRIdPTR "\n", i);
		void* pi = (void*) i;
		if ((ret = pthread_create(&threads[i], NULL, task_hello, pi))) {
			printf("pthread_create: error: %d\n", ret);
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < NUM_THREADS; ++i) {
		void *retval = NULL;
		if ((ret = pthread_join(threads[i], &retval) != 0)) {
			printf("pthread_join: error %d\n", ret);
		} else {
			printf("thread %" PRIdPTR " finished with return value %p\n", i, retval);
		}
	}
	pthread_exit(NULL);
}
