#include <unistd.h>

int main(int ac, char* const av[], char *const envp[]) {
	char *const arg[] = {"/usr/bin/python3", "./website/script/script.py", NULL};
	execve("/usr/bin/python3", arg, envp);
	return 1;
}
