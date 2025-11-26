#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 10

void *task_hello(void *data)
{
	int *intPtr = (int*)data;
	int i = *intPtr;
	free(intPtr);

	printf("                                        Hello world, data: %p, %d\n", data, i);


	int* ip = (int*)malloc(sizeof(int));
	*ip = 2 * i;
	pthread_exit(ip);
}

int main (int argc, char *argv[])
{
	pthread_t threads[NUM_THREADS];
	int ret;
	int i;
	for (i = 0; i < NUM_THREADS; ++i) {
		printf("main: pthread_create %d\n", i);
		int* ip = (int*)malloc(sizeof(int));
		*ip = i;
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
			if (retval) {
				printf("thread %d finished with return value %p (%d)\n", i, retval, *(int*)retval);
				free(retval);
			}
		}
	}
	pthread_exit(NULL);
}
