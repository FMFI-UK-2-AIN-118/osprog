#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 10

void *task_hello(void *data)
{
	// Following gives a warning because size of int is not / might not
	// be same as size of a pointer.
	// What is sizeof(int) and sizeof(void*) on your architecture?
	// pppppppp
	// ||||
	// vvvv
	// iiii
	//
	int i = (int)data;    // may case warning (pointer-to-int-cast)
	printf("                                        Hello world, data: %p, %d\n", data, i);
	// Same here
	pthread_exit((void*)(2*i));    // may cause warning (int-to-pointer-cast)
}

int main (int argc, char *argv[])
{
	pthread_t threads[NUM_THREADS];
	int ret;
	int i;
	for (i = 0; i < NUM_THREADS; ++i) {
		printf("main: pthread_create %d\n", i);
		// Following gives a warning because size of int is not / might not
		// be same as size of a pointer.
		//
		// iiii????
		// ||||||||
		// vvvvvvvv
		// pppppppp
		//
		void* pi = (void*) i;    // may cause warning (int-to-pointer-cast)
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
			printf("thread %d finished with return value %p\n", i, retval);
		}
	}
	pthread_exit(NULL);
}
