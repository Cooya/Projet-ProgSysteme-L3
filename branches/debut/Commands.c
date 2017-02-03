#include "Commands.h"
#include "EnvVars.h"
#include "Aliases.h"
#include "Shell.h"
#include <string.h>
#include <stdio.h>

#define NBFCTS 6

typedef struct _Command {
    char *name;
    commandFct ptr;
} Command;

Command commandsList[NBFCTS];
ShellParams *shellParams = NULL;

/****util*****/

char *boolToStr(bool val) {
	if(val)
		return "true";
	else
		return "false";
}

/*************/

void initCommands(ShellParams *params) {
	shellParams = params;
    initAliases();
    commandsList[0].name = "echo";
    commandsList[0].ptr  = cmdEcho;
    commandsList[1].name = "shellinfo";
    commandsList[1].ptr  = cmdShellinfo;
    commandsList[2].name = "printenv";
    commandsList[2].ptr  = cmdPrintenv;
    commandsList[3].name = "alias";
    commandsList[3].ptr  = cmdAlias;
    commandsList[4].name = "unalias";
    commandsList[4].ptr  = cmdUnalias;
    commandsList[5].name = "shellmode";
    commandsList[5].ptr  = cmdShellmode;
}

void uninitCommands() {
    uninitAliases();
}

/* Retourne le pointeur sur la fonction demandée ou NULL si elle n'existe pas */
commandFct getCommand(char *name) {
    for(int i=0; i<NBFCTS; i++)
        if(strcmp(commandsList[i].name, name) == 0)
            return commandsList[i].ptr;
    return NULL;
}


bool externalCommandExists(char *name) {
    return false;
}

int execExternalCommand(char **args) {
    return 0;
}

bool isParamArg(char *arg) {
	return *arg == '=';
}

char **parseArg(char *arg) {
	if(!isParamArg(arg))
		return NULL;
	
	char *ptr = arg;
	int argsLen = 0;
	do {
		ptr = strchr(ptr+1, ' ');
		argsLen++;
	} while(ptr);
	
	char **argArray = malloc((argsLen+1)*sizeof(char *));
	
	ptr = arg;
	for(int i=0; i<argsLen; i++) {
		argArray[i] = ptr+1;
		ptr = strchr(ptr+1, ' ');
		if(ptr)
			*ptr = '\0';
	}
	argArray[argsLen] = NULL;
	
	return argArray;
}

void unparseArg(char **argArray) {
	free(argArray);
}



int cmdEcho(char **args) {
    for(int i=1; args[i]; i++) {
        if(i==1)
            printf("%s", args[i]);
        else
            printf(" %s", args[i]);
    }
    printf("\n");
    return 0;
}

int cmdShellinfo(char **args) {
    printf("You are using Mini-Shell!\n");
    printf("Shell Paramaters :\n");
    printf("    verbose: %s\n", boolToStr(shellParams->verbose));
    return 0;
}

int cmdPrintenv(char **args)
{
    char **envp = getEnvVars();
    int i=0;
    while(envp[i])
    {
        printf("%s\n", envp[i]);
        i++;
    }
    return 0;
}

int cmdAlias(char **args) {
	int i=1;
	while(args[i]) {
		if(isParamArg(args[i])) {
			char **argArray = parseArg(args[i]);
			addAlias(argArray[0], argArray+1);
			unparseArg(argArray);
		}
		i++;
	}
    return 0;
}

int cmdUnalias(char **args) {
	int i=1;
	while(args[i]) {
		if(!isParamArg(args[i]))
			removeAlias(args[i]);
		i++;
	}
    return 0;
}

int cmdShellmode(char **args) {
	int i=1;
	while(args[i]) {
		if(isParamArg(args[i])) {
			char **argArray = parseArg(args[i]);
			if(strcmp(argArray[0], "verbose") == 0) {
				if(strcmp(argArray[1], "true") == 0)
					shellParams->verbose = true;
				else
					shellParams->verbose = false;
			}
			unparseArg(argArray);
		}
		else {
			if(strcmp(args[i], "verbose") == 0)
				shellParams->verbose = true;
		}
		i++;
	}
	return 0;
}
