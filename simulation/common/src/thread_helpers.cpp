#include "thread_helpers.h"

#include <time.h>

void thread_sleep_s(unsigned int seconds) {
  struct timespec ts;
  ts.tv_sec = seconds;
  ts.tv_nsec = 0;
  nanosleep(&ts, NULL);
}

void thread_sleep_ms(unsigned int milliseconds) {
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = milliseconds * 1000000;
  nanosleep(&ts, NULL);
}
