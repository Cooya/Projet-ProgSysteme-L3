#ifndef HISTORY_H
#define HISTORY_H

#include <stdbool.h>

void initHistory();
void uninitHistory();
void addToHistory(char*);
void printHistory();
char* getLastCommandFromHistory();
void emptyHistory();

#endif
