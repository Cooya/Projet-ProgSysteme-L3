#ifndef HISTORY_H
#define HISTORY_H

#include <stdbool.h>

#define UP true
#define DOWN false

void initHistory();
void uninitHistory();
void addToHistory(char*);
void printHistory();
char* getCommandFromHistory(bool);
void emptyHistory();

#endif
