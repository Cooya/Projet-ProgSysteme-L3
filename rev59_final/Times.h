#ifndef TIMES_H
#define TIMES_H

#include <sys/types.h>

void initTimes();
void uninitTimes();
void startTime(pid_t);
void finishTime(pid_t);
void printTimes();

#endif

