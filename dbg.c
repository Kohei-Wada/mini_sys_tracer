#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <errno.h>
#include <sys/wait.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/user.h>

#include "syscalls.h"
#include "syscallents.h"


#define offsetof(a, b) __builtin_offsetof(a, b)
#define get_reg(pid, name) __get_reg(pid, offsetof(struct user, regs.name))




long __get_reg(pid_t pid, int off)
{
    long val = ptrace(PTRACE_PEEKUSER, pid, off);
    assert(errno == 0);
    return val;
}



int do_child(int argc, char **argv)
{
char *args[argc+1];

    for(int i = 0; i < argc; ++i)
        args[i] = argv[i];

    args[argc] = NULL;

    ptrace(PTRACE_TRACEME);
    kill(getpid(), SIGSTOP);

    return execvp(args[0], args);
}




int wait_for_syscall(pid_t pid)
{
int status;
    while(1){
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        waitpid(pid, &status, 0);

        if(WIFSTOPPED(status) && WSTOPSIG(status) == (SIGTRAP | 0x80))
            return 0;
        if(WIFEXITED(status))
            return 1;

        fprintf(stderr, "stopped %d (%x)\n", status, WSTOPSIG(status));
    }
}



void print_syscall(pid_t pid)
{
    int num = get_reg(pid, orig_rax);
    assert(errno == 0);
    fprintf(stderr, "SYS_%s\n", syscalls[num].name);
}



int do_trace(pid_t pid)
{
int status;

    waitpid(pid, &status, 0);
    assert(WIFSTOPPED(status));
    ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACESYSGOOD);

    while(1){
        if(wait_for_syscall(pid) != 0)
            break;

        print_syscall(pid);

        if(wait_for_syscall(pid) != 0)
            break;

    }

    return 0;
}


void Usage(void)
{
    fprintf(stderr, "<USAGE>\n");
    fprintf(stderr, "    [ex] ./debug -a 1234\n");
    fprintf(stderr, "         => attach to pid 1234\n");
    fprintf(stderr, "         ./debug  [program]\n");
    fprintf(stderr, "         => fork new program\n");
}


int main(int argc, char **argv)
{
pid_t pid;

    if(argc < 2){
        Usage();
        return 1;
    }

    if(strcmp(argv[1], "-a") == 0){
        pid = atoi(argv[2]);

        if(ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1){
            perror("ptrace");
            return 1;
        }

        do_trace(pid);
    }

    else{
        pid = fork();
        if(pid > 0){
            return do_trace(pid);
        }
        else if(!pid){
            return do_child(argc-1, argv+1);
        }
        else if(pid == -1){
            perror("fork");
            return 1;
        }
    }
    return 0;
}
