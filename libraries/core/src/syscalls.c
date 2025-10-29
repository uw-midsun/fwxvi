/************************************************************************************************
 * @file   syscalls.c
 *
 * @brief  System call stubs for newlib
 *
 * @date   2025-01-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>

#undef errno
extern int errno;

/* Minimal environment */
char *__env[1] = { 0 };
char **environ = __env;

extern char _end;  /* Defined by the linker script */
extern char _estack;  /* Defined by the linker script */
static char *heap_ptr = NULL;

void *_sbrk(int incr) {
  char *prev_heap_ptr;

  if (heap_ptr == NULL) {
    heap_ptr = &_end;
  }

  prev_heap_ptr = heap_ptr;

  /* Check if we have enough space */
  if (heap_ptr + incr > &_estack) {
    errno = ENOMEM;
    return (void *)-1;
  }

  heap_ptr += incr;
  return (void *)prev_heap_ptr;
}

/* File operations - stub implementations */
int _close(int file) {
  (void)file;
  return -1;
}

int _fstat(int file, struct stat *st) {
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}

int _isatty(int file) {
  (void)file;
  return 1;  /* Pretend everything is a TTY */
}

int _lseek(int file, int ptr, int dir) {
  (void)file;
  (void)ptr;
  (void)dir;
  return 0;
}

int _read(int file, char *ptr, int len) {
  (void)file;
  (void)ptr;
  (void)len;
  return 0;
}

/* Process control stubs */
int _getpid(void) {
  return 1;
}

int _kill(int pid, int sig) {
  (void)pid;
  (void)sig;
  errno = EINVAL;
  return -1;
}

void _exit(int status) {
  (void)status;
  /* Infinite loop to halt execution */
  while (1) {
    __asm__("NOP");
  }
}

/* Additional stubs that might be needed */
int _open(const char *name, int flags, int mode) {
  (void)name;
  (void)flags;
  (void)mode;
  return -1;
}

int _link(char *old, char *new) {
  (void)old;
  (void)new;
  errno = EMLINK;
  return -1;
}

int _unlink(char *name) {
  (void)name;
  errno = ENOENT;
  return -1;
}

int _stat(const char *file, struct stat *st) {
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}

int _execve(char *name, char **argv, char **env) {
  (void)name;
  (void)argv;
  (void)env;
  errno = ENOMEM;
  return -1;
}

int _fork(void) {
  errno = EAGAIN;
  return -1;
}

int _wait(int *status) {
  (void)status;
  errno = ECHILD;
  return -1;
}

clock_t _times(struct tms *buf) {
  (void)buf;
  return -1;
}

int _gettimeofday(struct timeval *p, void *z) {
  (void)p;
  (void)z;
  return -1;
}
