#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "Commands.h"
#include "EnvVars.h"
#include "Aliases.h"
#include "History.h"
#include "Shell.h"
#include "extlib.dll.h"
#include "job.h"

#define NBFCTS 23

typedef struct _Command {
    char *name;
    commandFct ptr;
} Command;

static Command commandsList[NBFCTS];
static ShellParams *shellParams = NULL;
static Dll directoriesStack = NULL;

/****util*****/

char *boolToStr(bool val) {
	if(val)
		return "true";
	else
		return "false";
}

/*************/

void initCommands(ShellParams *params, char *pwd) {

	directoriesStack = dllNew(sizeof(char*));
	dllPushFront(directoriesStack, &pwd);

	shellParams = params;
    initAliases();
    initHistory();
    commandsList[0].name = "shellinfo";
    commandsList[0].ptr  = cmdShellinfo;
    commandsList[1].name = "shellmode";
    commandsList[1].ptr  = cmdShellmode;
    commandsList[2].name = "alias";
    commandsList[2].ptr  = cmdAlias;
    commandsList[3].name = "unalias";
    commandsList[3].ptr  = cmdUnalias;
    commandsList[4].name = "cd";
    commandsList[4].ptr  = cmdCd;
    commandsList[5].name = "dirs";
    commandsList[5].ptr  = cmdDirs;
    commandsList[6].name = "popd";
    commandsList[6].ptr  = cmdPopd;
    commandsList[7].name = "pushd";
    commandsList[7].ptr  = cmdPushd;
    commandsList[8].name = "echo";
    commandsList[8].ptr  = cmdEcho;
    commandsList[9].name = "exec";
    commandsList[9].ptr  = cmdExec;
    commandsList[10].name = "exit"; // jamais utilisée
    commandsList[10].ptr  = NULL;
    commandsList[11].name = "history";
    commandsList[11].ptr  = cmdHistory;
    commandsList[12].name = "kill";
    commandsList[12].ptr  = cmdKill;
    commandsList[13].name = "printenv";
    commandsList[13].ptr  = cmdPrintenv;
    commandsList[14].name = "pwd";
    commandsList[14].ptr  = cmdPwd;
    commandsList[15].name = "source";
    commandsList[15].ptr  = cmdSource;
    commandsList[16].name = "suspend";
    commandsList[16].ptr  = cmdSuspend;
    commandsList[17].name = "umask";
    commandsList[17].ptr  = cmdUmask;
    commandsList[18].name = "times";
    commandsList[18].ptr  = cmdTimes;
    commandsList[19].name = "wait";
    commandsList[19].ptr  = cmdWait;
  	commandsList[20].name = "jobs";
  	commandsList[20].ptr  = cmdJobs;
  	commandsList[21].name = "fg";
  	commandsList[21].ptr  = cmdFG;
  	commandsList[22].name = "bg";
  	commandsList[22].ptr  = cmdBG;
}

void uninitCommands() {
    uninitAliases();
    uninitHistory();
    dllDel(directoriesStack);
}

/* Retourne le pointeur sur la fonction demandée ou NULL si elle n'existe pas */
commandFct getCommand(char *name) {
    for(int i=0; i<NBFCTS; i++)
        if(strcmp(commandsList[i].name, name) == 0)
            return commandsList[i].ptr;
    return NULL;
}


bool externalCommandExists(char *name)
{
    char *env = strdup( getenv( "PATH" ) );
    char *ps = env;
    char *pp;
    bool accessOk = false;
    while( ( pp = strsep( &ps, ":" ) ) )
    {
        char binfn[256];
        sprintf( binfn, "%s/%s", pp, name );
        if( access( binfn, X_OK ) == 0 )
        {
            accessOk = true;
            break;
        }
    }
    free( env );
    return accessOk;
}

int execExternalCommand(char **args) {

	pid_t pid=fork();
	int status;

	if(pid == 0){ // child process
		execvp(args[0],args);
		perror("execExternalCommand");
		return EXIT_FAILURE;
	}
	else if(pid >0){
		waitpid(pid, &status, 0);
	}
	else{
		perror("pid error execExternalCommand");
	}



	if(WIFEXITED(status)){
		return status; //printf("child exit code : %d", status);
	}
	else {
		return EXIT_FAILURE;
	}
	//wait(0);
	return EXIT_FAILURE;


}

/*bool isParamArg(char *arg) {
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
}*/

int getArgsLen(char **args) {
    int i=0;
    while(args[i]) i++;
    return i;
}

void printDirectoriesStack() {
	DllNode node = dllGetFront(directoriesStack);
	while (node!=NULL) {
		printf("%s ",*((char**) dllGetData(node)));
		node = dllGetNext(node);
	}
	printf("\n");
}

int cmdShellinfo(char **args) {
    printf("You are using Mini-Shell!\n");
    printf("Shell Paramaters :\n");
    printf("    verbose: %s\n", boolToStr(shellParams->verbose));
    return 0;
}

/*int cmdShellmode(char **args) {
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
}*/

int cmdShellmode(char **args) {
    if(!args[1] || !args[2])
        return -1;
    if(strcmp(args[1], "verbose") == 0) {
        if(strcmp(args[2], "true") == 0)
            shellParams->verbose = true;
        else if(strcmp(args[2], "false") == 0)
            shellParams->verbose = false;
        else
            return -1;
	}
	else
        return -1;
	return 0;
}

/*int cmdAlias(char **args) {
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
}*/

int cmdAlias(char **args) {
    if(!args[1] || !args[2])
        return -1;
	addAlias(args[1], args+2);
    return 0;
}

/*int cmdUnalias(char **args) {
	int i=1;
	while(args[i]) {
		if(!isParamArg(args[i]))
			removeAlias(args[i]);
		i++;
	}
    return 0;
}*/

int cmdUnalias(char **args) {
    if(!args[1])
        return -1;
    removeAlias(args[1]);
    return 0;
}

// -------------- PARTIE EN TRAVAUX ----------------------------------

int cmdCd(char **args) { // fonction terminée mais foireuse
	char* dest;
	if (args[1]==NULL)
		dest = getEnvVar("HOME");
	else
		dest = args[1];

	if (chdir(dest)==-1) {
		printf("%s\n",strerror(errno));
		return 0;
	}
	dllPopFront(directoriesStack);
	dllPushFront(directoriesStack, &dest);
	return 0;
}

int cmdDirs(char **args) { // fonction terminée
	printDirectoriesStack();
	return 0;
}

int cmdPopd(char **args) { // fonction pas terminée
	if(dllCount(directoriesStack)==1)
		printf("Directories stack empty\n");
	else {
		// chdir(*((char**) dllGetData(dllGetFront(directoriesStack))));
		printDirectoriesStack();
	}
	return 0;
}

int cmdPushd(char **args) { // fonction pas terminée
	if(args[1]==NULL) {
		printf("Too few arguments (1 required)\n");
		return -1;
	}
	else {
		if(chdir(args[1])==-1) {
			printf("%s\n", strerror(errno));
			return 0;
		}
		dllPushFront(directoriesStack, &args[1]);
	}
	printDirectoriesStack();
	return 0;
}

// ----------------- FIN PARTIE EN TAVAUX ----------------------------

int cmdEcho(char **args) {
    for(int i=1; args[i]; i++) {
    	char *ps = args[i];
    	if(ps[0]=='$'){
    		ps = getenv(ps+1);
    		if(!ps)
    			ps="";
    	}
        if(i==1)
            printf("%s", ps);
        else
            printf(" %s", ps);
    }
    printf("\n");
    return 0;
}

int cmdExec(char **args) {
	printf("Command not implemented yet\n");
	return 0;
}

int cmdHistory(char **args) {
	if (args[1]!=NULL)
		if(args[1][0]=='-' && args[1][1]=='c')
			emptyHistory();
		else {
			printf("Syntax error\n");
			return -1;
		}
	else
		printHistory();
	return 0;
}

int cmdKill(char **args) {
	if(args[1]==NULL) {
		printf("Too few aguments (at least 1 required)\n");
		return -1;
	}
	int sig;
	int pid;
	if (args[1][0]=='-') {
		if (args[2]!=NULL) {
			sig = atoi(&args[1][1]);
			pid = atoi(args[2]);
		}
		else {
			printf("Syntax error\n");
			return -1;
		}
	}
	else if (args[2]!=NULL && args[2][0]=='-') {
		sig = atoi(&args[2][1]);
		pid = atoi(args[1]);
	}
	else {
		sig = SIGTERM;
		pid = atoi(args[1]);
	}
	if (kill(pid, sig)==-1)
		printf("%s\n",strerror(errno));
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

int cmdPwd(char **args) {
	printf("%s\n",getEnvVar("PWD"));
	return 0;
}

int cmdSource(char **args) {
	printf("Command not implemented yet\n");
	return 0;
}

int cmdSuspend(char **args) {
	sigset_t set;
	sigfillset(&set);
	sigprocmask(SIG_SETMASK,&set,NULL);
	while(1){}
	return 0;
}

int cmdUmask(char **args) {
	printf("Command not implemented yet\n");
	return 0;
}

int cmdTimes(char **args) {
	printf("Command not implemented yet\n");
	return 0;
}

int cmdWait(char **args) {
	if(args[1]==NULL) {
		printf("Too few arguments (1 required)\n");
		return -1;
	}
	if(waitpid(atoi(args[1]),NULL,0)==-1)
		printf("Unknown PID\n");
	return 0;
}

int cmdJobs(char **args) {
   return job_jobs();
}

int cmdFG( char **args ) {
   int n = args[1] ? atol( args[1] ) : job_total()-1;
   return job_fg( n );
}

int cmdBG( char **args ) {
   int n = args[1] ? atol( args[1] ) : job_current();
   return job_bg( n );
}
