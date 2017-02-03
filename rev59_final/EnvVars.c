#include "EnvVars.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char **envp = NULL;

void storeEnvVars(char **arr) {
    envp = arr;
}

char *getEnvVar(char *name) {
    int nameLength = strlen(name);

    for(int i=0; envp[i]; i++)
        if(strncmp(envp[i], name, nameLength) == 0) //si le nom correspond
            return strchr(envp[i]+nameLength, '=')+1; //on renvoie la chaine après le caractère '='

    return NULL;
}

char **getEnvVars() {
    return envp;
}

char* replaceTilde(char* input) { // fait une allocation
	if(input[0]=='~') {
		char* home = getEnvVar("HOME");
		char* output = malloc(strlen(input) + strlen(home));
		output[0] = '\0';
		strcat(output,home);
		strcat(output,input+1);
		return output;	
	}
	else
		return NULL;
}

void printPathWithTilde(char* str) {
	char* home = getEnvVar("HOME");
	int homeSize = strlen(home);
	if(strncmp(str, home, homeSize) == 0)
		printf("%c%s", '~', str+homeSize);
	else
		printf("%s", str);
}
