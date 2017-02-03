#ifndef ALIASES_H
#define ALIASES_H

#include <stdbool.h>
#include "CommandType.h"

void initAliases();
void uninitAliases();

void addAlias(char *alias, char **target);
char **getAlias(char *alias);
void removeAlias(char *alias);

#endif // ALIASES_H
