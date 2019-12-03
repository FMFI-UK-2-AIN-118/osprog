/**
 * A simple shell example.
 *
 * Use "gcc -oshell -Wall shell.c" to compile.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * Parse command from cmdline into the array
 * argv. Note that strtok modifies cmdline!
 * It stores zeroes in the cmdline array,
 * thus we end up with "multiple strings" in
 * cmdline with pointers from argv pointing at
 * them.
 */
int parse_command(char *cmdline, char * argv[])
{
	char *saveptr = NULL;
	int i = 0;
	while (1) {
		argv[i] = strtok_r(cmdline, " \t\n", &saveptr);
		cmdline = NULL;
		if (argv[i] == NULL)
			return i;
		++i;
	}
}

void readline(char *s, int size)
{
	printf("> ");
	if (!fgets(s, size, stdin)) {
		perror("fgets");
		exit(EXIT_FAILURE);
	}
}

void run_command(int n, char *const argv[])
{
	pid_t pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	else if (pid > 0) {
		// parent
		printf("= started [%d] %d %s\n", n, pid, argv[0]);

		int wstatus;
		if (waitpid(pid, &wstatus, 0) == -1) {
			perror("waitpid");
			exit(EXIT_FAILURE);
		}

		printf("= finished [%d] %d %s ", n, pid, argv[0]);
		if (WIFEXITED(wstatus)) {
			printf("%d\n", WEXITSTATUS(wstatus));
		}
		else if(WIFSIGNALED(wstatus)) {
			printf("terminated by signal %d\n", WTERMSIG(wstatus));
		}
		else {
			printf("ERR\n");
		}
	}
	else {
		// child
		// we should close any file descriptors etc here if we had opened any

		// we use execvp:
		// v - pass args as array
		// p - search in path for executable

		execvp(argv[0], argv);
		// if the call finishes, it's an error
		perror("exec");
		exit(EXIT_FAILURE);
	}
}

int main()
{
	char cmdline[256];
	char * cmdargv[11];
	int n = 0;
	while (1) {
		readline(cmdline, sizeof(cmdline));
		int num_args = parse_command(cmdline, cmdargv);

		if (num_args == 0)
			continue; // empty command

		char *cmd = cmdargv[0];
		if (!strcmp("exit", cmd)) {
			return 0;
		}

		++n;

		run_command(n, cmdargv);
	}
}
