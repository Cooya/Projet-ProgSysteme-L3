#ifndef ENVVARS_H
#define ENVVARS_H

void storeEnvVars(char**);

char *getEnvVar(char*);
char **getEnvVars();
char *replaceTilde(char*);
void printPathWithTilde(char*);

#endif // ENVVARS_H
