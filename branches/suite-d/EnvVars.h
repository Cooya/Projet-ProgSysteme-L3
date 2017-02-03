#ifndef ENVVARS_H
#define ENVVARS_H

void storeEnvVars(char **envp);

char *getEnvVar(char *name);
char **getEnvVars();
char *checkTilde(char *arg);

#endif // ENVVARS_H
