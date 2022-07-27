// 
// gcc ns_example.c -o ns_example
// 

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#include <unistd.h>

typedef unsigned int bool;
#define TRUE 1
#define FALSE 0

#define PID_NAMESPACE_PERM (CLONE_NEWPID | CLONE_NEWNS)
#define OS_RELEASE_INFO_FILE ("/etc/os-release")

static int pid_fd_open(pid_t pid, unsigned int flags) {
  int pid_fd = syscall(SYS_pidfd_open, pid, flags);
  if (pid_fd == -1) {
    fprintf(stderr, "!pid_fd_open %d\n", errno);
    exit(EXIT_FAILURE);
  }
  return pid_fd;
}

static void switch_namespace(int pid) {
  int ret = setns(pid, PID_NAMESPACE_PERM);
  if (ret) {
    fprintf(stderr, "!setns errno %d\n", errno);
    exit(EXIT_FAILURE);
  }
}

#define HOSTNAME_LEN 1024
#define BUF_LEN 1024

static void print_os_info(void) {
  char hostname[HOSTNAME_LEN] = {0};

  int ret = gethostname(hostname, sizeof(hostname));
  if (ret != 0) {
    fprintf(stderr, "!gethostname failed errno %d\n", errno);
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "hostname = %s\n", hostname);

  int err;
  char str[BUF_LEN] = {0};

  FILE *fp = fopen(OS_RELEASE_INFO_FILE, "r");
  if (!fp) {
    fprintf(stderr, "!fopen failed errno %d\n", errno);
    exit(EXIT_FAILURE);
  }

  size_t fret = fread(&str, sizeof(char), BUF_LEN, fp);
  if (fret == 0) {
    fprintf(stderr, "!fread failed errno %d\n", errno);
    exit(EXIT_FAILURE);
  }

  err = fclose(fp);
  if (err) {
    fprintf(stderr, "!fopen failed errno %d\n", errno);
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "OS information (%s):\n%s", OS_RELEASE_INFO_FILE, str);
}

long strtol_error(const char *str) {
  char *end;
  errno = 0;
  long ret = strtol(str, &end, 10);
  if (end == str || *end != '\0' ||
     ((ret == LONG_MIN || ret == LONG_MAX) && errno == ERANGE)) {
    fprintf(stderr, "!strtol %d\n", errno);
    exit(EXIT_FAILURE);
  }
  return ret;
}

// static void create_tmp_file(void) {
//   int fd = shm_open("/test", O_RDWR|O_CREAT, S_IRUSR|S_IRGRP|S_IROTH);
//   if (fd == -1) {
//       perror("shm_open() failed");
//       return EXIT_FAILURE;
//   } else {
//     fprintf(stderr, "success");
//     close(fd);
//   }
// }

// Main

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s [NamespaceProcPid] (additional process)\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  bool exec_run = (argc == 2) ? FALSE : TRUE;

  long pid = strtol_error(argv[1]);

  int pid_fd = pid_fd_open(pid, 0);

  print_os_info();

  fprintf(stderr, "\n\n ***switch namespace started*** \n\n");

  switch_namespace(pid_fd);

  print_os_info();

  // Run additional process
  if (exec_run == TRUE) {
    return execvp(argv[2], &argv[2]);
  }

  return EXIT_SUCCESS;
}
