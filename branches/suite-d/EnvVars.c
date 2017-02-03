#include "EnvVars.h"
#include <stdlib.h>
#include <string.h>

char **envp = NULL;

void storeEnvVars(char **arr) {
    envp = arr;
}

char *getEnvVar(char *name) {
    int nameLength = strlen(name);

    for(int i=0; envp[i]; i++)
        if(strncmp(envp[i], name, nameLength) == 0) //si le nom correspond
            return strchr(envp[i]+nameLength, '=')+1; //on renvoie la chaine apres le caractère '='

    return NULL;
}

char **getEnvVars() {
    return envp;
}

char* checkTilde(char* arg) {
	if(arg[0]=='~')
		return strcat(getEnvVar("HOME"),arg+1);		
	return arg;	
}
