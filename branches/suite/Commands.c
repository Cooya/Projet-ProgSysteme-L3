#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <math.h>

#include "Commands.h"
#include "EnvVars.h"
#include "Aliases.h"
#include "History.h"
#include "Times.h"
#include "Shell.h"
#include "extlib.dll.h"
#include "job.h"
#include "y.tab.h"

#define NBFCTS 24

typedef struct _Command
{
    char *name;
    commandFct ptr;
} Command;

static Command commandsList[NBFCTS];
static ShellParams *shellParams = NULL;
static Dll directoriesStack = NULL;
static bool isCmdExec = false;

/****util*****/

char *boolToStr(bool val)
{
    if(val)
        return "true";
    else
        return "false";
}

static bool isOctalNumber(char* s) {
	while(*s!='\0') {
		if(*s<48 || *s>55)
			return false;
		s++;
	}
	return true;
}

static int octalToDecimal(int n)
{
    int decimal=0, i=0, rem;
    while (n!=0) {
        rem = n%10;
        n/=10;
        decimal += rem*pow(8,i);
        ++i;
    }
    return decimal;
}

/*
static int getArgsLen(char **args) {
    int i=0;
    while(args[i]) i++;
    return i;
}
*/

static void printDirectoriesStack() {
	DllNode node = dllGetFront(directoriesStack);
	while (node!=NULL) {
		char* path = *((char**) dllGetData(node));
		printPathWithTilde(path);
		printf(" ");
		node = dllGetNext(node);
	}
	printf("\n");
}

static char* mallocPathForStack(char* path) {
	char* element = malloc(strlen(path)+1);
	element[0] = '\0';
	strcat(element, path);
	return element;
}

static void freeDirectoriesStack() {
	DllNode node = dllGetFront(directoriesStack);
	while (node!=NULL) {
		free(*((char**) dllGetData(node)));
		node = dllGetNext(node);
	}
}

/*************/

void initCommands(ShellParams *params, char *pwd) {
	directoriesStack = dllNew(sizeof(char*));
	char* path = mallocPathForStack(pwd);
	dllPushFront(directoriesStack, &path);

    shellParams = params;
    initAliases();
    initHistory();
    initTimes();
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
    commandsList[23].name = "disown";
    commandsList[23].ptr  = cmdDisown;
}

void uninitCommands() {
    uninitAliases();
    uninitHistory();
    uninitTimes();
    freeDirectoriesStack();
    dllDel(directoriesStack);
}

/* Retourne le pointeur sur la fonction demandée ou NULL si elle n'existe pas */
commandFct getCommand(char *name) {
    for(int i=0; i<NBFCTS; i++)
        if(strcmp(commandsList[i].name, name) == 0)
            return commandsList[i].ptr;
    return NULL;
}

char *externalCommandExists(char *name)
{
    bool accessOk = false;
    char *binfn=malloc(256*sizeof(char));

    if(name[0]== '.' && name[1]=='/')
    {
        sprintf(binfn, "%s/%s", getenv("PWD"), name+2);
        if (access (binfn, X_OK) == 0)
        {
            accessOk = true;
        }
    }
    else if(name[0]=='/')
    {
        sprintf(binfn,"%s", name);
        if( access(binfn , X_OK ) == 0)
        {
            accessOk=true;
        }
    }
    else
    {
        char *env = strdup( getenv( "PATH" ) );
        char *ps = env;
        char *pp;
        while( ( pp = strsep( &ps, ":" ) ) )
        {
            sprintf( binfn, "%s/%s", pp, name );
            if( access( binfn, X_OK ) == 0 )
            {
                accessOk = true;
                break;
            }
        }
        free( env );
    }
    if(accessOk){
        return binfn;
    }

    free(binfn);
    return NULL;
}


int execExternalCommand(char **args)
{
    int pid = fork();
#ifdef DEBUG
    fprintf( stderr, "%s( %p )\n", __FUNCTION__, args );
    fflush( stderr );
#endif
    if( pid < 0 )
        return errno;
    if( pid == 0 )
    {
        execvp( args[0], args );
        exit( -1 ); /* no such file or directory */
    }
    else
    {
		startTime(pid);
        job_add_pid( job_current(), pid, args[0] );
        //int exitCode = job_wait( job_current());
        //return exitCode;
        
        if(isCmdExec)
			waitpid(pid, NULL , 0);
    }
    return 0;


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
    if(strcmp(args[1], "verbose") == 0)
    {
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

int cmdCd(char **args) {
	char* dest;
	if (args[1] == NULL)
		dest = getEnvVar("HOME");
	else
		dest = args[1];
	if (chdir(dest)==-1) {
		printf("%s\n",strerror(errno));
		return 0;
	}
	dest = getcwd(NULL,0);
	char* previous = *((char**) dllGetData(dllGetFront(directoriesStack)));
	if (strcmp(previous,dest)) {
		free(previous);
		dllPopFront(directoriesStack);
		char* path = mallocPathForStack(dest);
		dllPushFront(directoriesStack, &path);
	}
	free(dest);
	return 0;
}

int cmdDirs(char **args) {
	printDirectoriesStack();
	return 0;
}

int cmdPopd(char **args) {
	char* path = *((char**) dllGetData(dllGetFront(directoriesStack)));
	if (chdir(path)==-1) {
		printf("%s\n", strerror(errno));
		return 0;
	}
	if(dllCount(directoriesStack)==1)
		printf("Directories stack empty\n");
	else {
		free(path);
		dllPopFront(directoriesStack);
		printDirectoriesStack();
	}
	return 0;
}

int cmdPushd(char **args) {
	if(args[1]==NULL) {
		printf("Too few arguments (1 required)\n");
		return -1;
	}
	else {
		if(chdir(args[1])==-1) {
			printf("%s\n", strerror(errno));
			return 0;
		}
		char* cwd = getcwd(NULL,0);
		dllPushFront(directoriesStack, &cwd);
		printDirectoriesStack();
	}
	return 0;
}

extern int lastExitCode;

int cmdEcho(char **args)
{
    for(int i=1; args[i]; i++)
    {
        char *ps = args[i];
        if(ps[0]=='$')
        {
            if( ps[1] == '?' )
            {
                static char lEC[32];
                sprintf( lEC, "%d", lastExitCode );
                ps = lEC;
            }
            else
            {
                ps = getenv(ps+1);
                if(!ps)
                    ps="";
            }
        }
        if(i==1)
            printf("%s", ps);
        else
            printf(" %s", ps);
    }
    printf("\n");
    return 0;
}

extern bool isBG;
int cmdExec(char **args)
{		
    int exitCode;
	commandFct fct = getCommand(args[1]);

	if(fct) {
		exitCode = fct(args+1); // commande interne
		if(!isBG)
			exit(EXIT_SUCCESS);
	}
	else {
		char *binfn = externalCommandExists(args[1]);
		if(binfn != NULL) {
			free(args[1]);
			args[1] = binfn;
			isCmdExec = true;
			exitCode = execExternalCommand(args+1);
			isCmdExec = false;
			if(!isBG)
				exit(EXIT_SUCCESS);
		}
		else {
			printf("%s: command not found\n", args[1]);
			exitCode = -1;
		}
	}
    return exitCode;
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
    if(args[1]==NULL)
    {
        printf("Too few aguments (at least 1 required)\n");
        return -1;
    }
    int sig;
    int pid;
    if (args[1][0]=='-')
    {
        if (args[2]!=NULL)
        {
            sig = atoi(&args[1][1]);
            pid = atoi(args[2]);
        }
        else
        {
            printf("Syntax error\n");
            return -1;
        }
    }
    else if (args[2]!=NULL && args[2][0]=='-')
    {
        sig = atoi(&args[2][1]);
        pid = atoi(args[1]);
    }
    else
    {
        sig = SIGTERM;
        pid = atoi(args[1]);
    }
    if (kill(pid, sig)==-1)
        printf("%s\n",strerror(errno));
    return 0;
}

int cmdPrintenv(char **args) {
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
    char* wd = getcwd(NULL,0);
	printf("%s\n",wd);
	free(wd);
	return 0;
}

int cmdSource(char **args) {
    extern bool exitAsked;

    if(!args[1])
        return EXIT_FAILURE;

    FILE * fsource = fopen(args[1], "r");

    if(!fsource)
    	return EXIT_FAILURE;

    extern FILE *yyin;

    FILE *yyinSave = yyin;
    yyin = fsource;
    EOFFct = EndOfFileWormsHole;
    int exitCode=-1;
    while(!feof(fsource)) {
    //fprintf(stderr, "DEB\n");
    	if(yyparse()==0) {
    	//fprintf(stderr, "MIL\n");
    	    Expression *e = ExpressionAnalysee;
    	    exitCode = executeExpression(e); //prepareExpression(e);
    	    expression_free(e);
	    if(exitAsked)
    	    	break;
    	}
    	else {
            /* L'analyse de la ligne de commande a donné une erreur */
            fprintf (stderr,"Expression syntaxiquement incorrecte !\n");
            reset_command_line();
        }
        //fprintf(stderr, "FIN\n");
    }

    EOFFct = EndOfFile;
    yyin = yyinSave;
    fclose(fsource);
    return exitCode;
}

void empty(int i) {}
int cmdSuspend(char **args) {
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set,SIGCONT);
    struct sigaction s;
    s.sa_handler = empty;
    sigaction(SIGCONT, &s, NULL);
    sigsuspend(&set);
    return 0;
}

int cmdUmask(char **args) {
	if(args[1]) {
		if (!isOctalNumber(args[1])) {
			printf("Bad octal number\n");
			return -1;
		}
		mode_t mask = octalToDecimal(atoi(args[1]));
		umask(mask);
	}
	else {
		mode_t mask = umask(0);
		umask(mask);
		printf("%o\n", mask);
	}
	return 0;
}

int cmdTimes(char **args) {
	printTimes();
	return 0;
}

int cmdWait(char **args) {
    if(args[1]==NULL)
    {
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

int cmdFG(char **args) {
	int n = args[1] ? atoi( args[1] ) : job_total()-1;
    if(n<0)
    {
        fprintf(stderr, "Wrong job number\n");
        return EXIT_FAILURE;
    }    return job_fg( n );
}

int cmdBG(char **args) {
    int n = args[1] ? atoi( args[1] ) : job_current();
    if(n<0)
    {
        fprintf(stderr, "Wrong job number\n");
        return EXIT_FAILURE;
    }
    return job_bg( n );
}

int cmdDisown(char **args) {
    int n = args[1] ? atoi( args[1] ) : job_current();
    if(n<0)
    {
        fprintf(stderr, "Wrong job number\n");
        return EXIT_FAILURE;
    }

    return job_disown( n );
}
