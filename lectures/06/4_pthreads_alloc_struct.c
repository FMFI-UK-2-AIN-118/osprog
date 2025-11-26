#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 10

struct task_args {
	int input;
	int output;
};

void *task_hello(void *data)
{
	struct task_args* args = (struct task_args*)data;

	printf("                                        Hello world, data: %p, %d\n", data, args->input);

	args->output = 2 * args->input;
	pthread_exit(data);
}

int main (int argc, char *argv[])
{
	pthread_t threads[NUM_THREADS];
	int ret;
	int i;
	for (i = 0; i < NUM_THREADS; ++i) {
		printf("main: pthread_create %d\n", i);
		struct task_args* args = (struct task_args*)malloc(sizeof(struct task_args));
		args->input = i;
		if ((ret = pthread_create(&threads[i], NULL, task_hello, args))) {
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
				struct task_args* args = (struct task_args*)retval;
				printf("thread %d finished with return value %p (%d)\n", i, retval, args->output);
				free(args);
			}
		}
	}
	pthread_exit(NULL);
}
