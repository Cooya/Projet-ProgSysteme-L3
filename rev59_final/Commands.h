#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include "CommandType.h"
#include "Shell.h"

void initCommands(ShellParams *params, char *pwd);
void uninitCommands();

commandFct getCommand(char *name);

char* externalCommandExists(char *name);
int execExternalCommand(char **args);

int cmdShellinfo(char **args);
int cmdShellmode(char **args);

int cmdAlias(char **args);
int cmdUnalias(char **args);
int cmdCd(char **args);
int cmdDirs(char **args);
int cmdPopd(char **args);
int cmdPushd(char **args);
int cmdEcho(char **args);
int cmdExec(char **args);
int cmdExit(char **args);
int cmdHistory(char **args);
int cmdKill(char **args);
int cmdPrintenv(char **args);
int cmdPwd(char **args);
int cmdSource(char **args);
int cmdSuspend(char **args);
int cmdUmask(char **args);
int cmdTimes(char **args);
int cmdWait(char **args);

int cmdJobs(char **args);
int cmdFG(char **args);
int cmdBG(char **args);
int cmdDisown(char **args);

#endif // COMMANDS_H
