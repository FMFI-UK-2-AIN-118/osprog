#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 10

void *task_hello(void *data)
{
	int *intPtr = (int*)data;
	int sum = 0;
	for (int i = 0; i < 1000000; ++i) {
		sum += i;
	}
	printf("                                        Hello world, data: %p, %d\n", data, *intPtr);
	pthread_exit(data);
}

int main (int argc, char *argv[])
{
	pthread_t threads[NUM_THREADS];
	int ret;
	int i;
	for (i = 0; i < NUM_THREADS; ++i) {
		printf("main: pthread_create %d\n", i);
		int* ip = &i;
		if ((ret = pthread_create(&threads[i], NULL, task_hello, ip))) {
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
