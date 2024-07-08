#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
    return 1;
  }

  pid_t pid = atoi(argv[1]);

  // Attach to the process with the given PID and initiate tracing (sends a
  // SIGSTOP) on the tracee to halt its execution.
  if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
    perror("ptrace attach");
    return 1;
  }

  // Wait for the tracee to stop and become ready for further tracing.
  waitpid(pid, 0, 0);

  for (;;) {
    // Restart the tracee and stop at the next system call entry or exit. Here,
    // we enter the syscall.
    if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1) {
      perror("ptrace syscall enter");
      break;
    }
    waitpid(pid, 0, 0);

    // Retrieve the tracee's register values.
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) {
      perror("ptrace getregs");
      break;
    }

    // Check if the syscall being traced is SYS_getrandom.
    int intercepted = 0;
    if (regs.orig_rax == SYS_getrandom) {
      intercepted = 1;
    }

    // Exit the syscall and wait for the tracee to stop again.
    if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1) {
      perror("ptrace syscall exit");
      break;
    }
    waitpid(pid, 0, 0);

    if (intercepted) {
      fprintf(stderr,
              "intercepted getrandom call: regs.rdi = %llu, regs.rsi = %zu\n",
              regs.rdi, regs.rsi);

      unsigned long long buf = regs.rdi;
      size_t count = regs.rsi;

      // Overwrite the buffer contents with zeroes.
      for (size_t i = 0; i < count; i += sizeof(long)) {
        if (ptrace(PTRACE_POKEDATA, pid, buf + i, 0) == -1) {
          perror("ptrace pokedata");
          break;
        }
      }

      // Set the return value to indicate the amount of data written.
      regs.rax = count;

      // Modify the tracee's registers to reflect the changes made.
      if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1) {
        perror("ptrace setregs");
        break;
      }
    }
  }

  ptrace(PTRACE_DETACH, pid, NULL, NULL);
  return 0;
}
