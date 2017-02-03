#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include "CommandType.h"

void initCommands();
void uninitCommands();

commandFct getCommand(char *name);

bool externalCommandExists(char *name);
int execExternalCommand(char **args);


int cmdEcho(char **args);
int cmdShellinfo(char **args);
int cmdPrintenv(char **args);
int cmdAlias(char **args);
int cmdUnalias(char **args);
int cmdShellmode(char **args);

#endif // COMMANDS_H
