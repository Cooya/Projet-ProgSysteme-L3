#define _GNU_SOURCE

/* Construction des arbres représentant des commandes */

#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include "Shell.h"
#include "Commands.h"
#include "Aliases.h"
#include "History.h"
#include "Times.h"
#include "EnvVars.h"
#include "job.h"

//Global variables
bool exitAsked = false;
static ShellParams shellParams;
bool isBG = false;

/*
 * Construit une expression à partir de sous-expressions
 */
Expression *ConstruireNoeud (expr_t type, Expression *g, Expression *d, char **args)
{
    Expression *e;

    if ((e = (Expression *)malloc(sizeof(Expression))) == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    e->type   = type;
    e->gauche = g;
    e->droite = d;
    e->arguments = args;
    return e;
} /* ConstruireNoeud */

/*
 * Renvoie la longueur d'une liste d'arguments
 */
int LongueurListe(char **l)
{
    char **p;
    for (p=l; *p != NULL; p++)
        ;
    return p-l;
} /* LongueurListe */

/*
 * Renvoie une liste d'arguments, la première case étant initialisée à NULL, la
 * liste pouvant contenir NB_ARGS arguments (plus le pointeur NULL de fin de
 * liste)
 */
char **InitialiserListeArguments (void)
{
    char **l;

    l = (char **) (calloc (NB_ARGS+1, sizeof (char *)));
    *l = NULL;
    return l;
} /* InitialiserListeArguments */



/*
 * Ajoute en fin de liste le nouvel argument et renvoie la liste résultante
 */
char **AjouterArg (char **Liste, char *Arg)
{
    char **l;

    Arg = checkTilde(Arg);

    l = Liste + LongueurListe (Liste);
    *l = (char *) (malloc (1+strlen (Arg)));
    strcpy (*l++, Arg);
    *l = NULL;
    return Liste;
} /* AjouterArg */



/*
 * Fonction appelée lorsque l'utilisateur tape "".
 */
void EndOfFile ()
{
    exit (0);//exitAsked = true;
} /* EndOfFile */

void EndOfFileWormsHole() {
}

/*
 * Appelée par yyparse() sur erreur syntaxique
 */
void yyerror (char *s)
{
    fprintf(stderr, "%s\n", s);
}


void
expression_free(Expression *e)
{
    if (e == NULL)
        return;

    expression_free(e->gauche);
    expression_free(e->droite);

    if (e->arguments != NULL)
    {
        for (int i = 0; e->arguments[i] != NULL; i++)
            free(e->arguments[i]);
        free(e->arguments);
    }

    free(e);
}


// Taking signal number and new signal handler function
void (*customize_signal( int signo, void (*hndlr)( int ) ) )( int )
{
    struct sigaction a, prev_a;
    a.sa_handler = hndlr;
    sigemptyset( &a.sa_mask );
    a.sa_flags=0;
    if( signo != SIGALRM )
        a.sa_flags |= SA_RESTART;
    if( sigaction( signo, &a, &prev_a ) < 0 )
    {
        fprintf(stderr, "SIG_ERR\n");
        return SIG_ERR;
    }
    return prev_a.sa_handler;
}

int ctrl_c_flag = 0;
int ctrl_z_flag = 0;

void ctrl_c_pressed( int s )
{
    //fprintf(stderr, "job_interrupt( %d ) \n",job_current());
    ctrl_c_flag = 1;
    //job_interrupt( job_current() );
}


void ctrl_z_pressed( int s )
{
    ctrl_z_flag = 1;
}


int lastExitCode=0;
int main (int argc, char **argv, char **envp)
{
    EOFFct = EndOfFile;
    shellParams.verbose = false;

    storeEnvVars(envp);

    char hostName[256]; // Longueur maximale pour un nom d'ordinateur
    if(!getHostName(hostName))
        fprintf(stderr, "Error: Hostname not found.\n");

    char *logName = getEnvVar("LOGNAME");
    char *pwd = getEnvVar("PWD");

    initCommands(&shellParams, pwd);

    customize_signal( SIGINT, ctrl_c_pressed );
    //customize_signal( SIGSTOP, ctrl_z_pressed );
    customize_signal( SIGTSTP, ctrl_z_pressed );

    while (1)
    {
        pwd=getcwd(NULL,0);
        printf("%s@%s:%s# ", logName, hostName, pwd);
        free(pwd);
        fflush(stdout);
        isBG=false;

        if (yyparse () == 0)
        {
            /*--------------------------------------------------------------------------------------.
            | L'analyse de la ligne de commande est effectuée sans erreur.  La variable globale     |
            |       ExpressionAnalysee pointe sur un arbre représentant l'expression.  Le type      |
            |       "Expression" de l'arbre est décrit dans le fichier Shell.h. Il contient 4       |
            |       champs. Si e est du type Expression :                         |
            |                                             |
            | - e.type est un type d'expression, contenant une valeur définie par énumération dans  |
            |   Shell.h. Cette valeur peut être :                                 |
            |                                             |
            |   - VIDE, commande vide                                     |
            |   - SIMPLE, commande simple et ses arguments                        |
            |   - SEQUENCE, séquence (;) d'instructions                       |
            |   - SEQUENCE_ET, séquence conditionnelle (&&) d'instructions                |
            |   - SEQUENCE_OU, séquence conditionnelle (||) d'instructions                |
            |   - BG, tâche en arrière plan (&)                           |
            |   - PIPE, pipe (|).                                     |
            |   - REDIRECTION_I, redirection de l'entrée (<)                          |
            |   - REDIRECTION_O, redirection de la sortie (>)                         |
            |   - REDIRECTION_A, redirection de la sortie en mode APPEND (>>).            |
            |   - REDIRECTION_E, redirection de la sortie erreur,                     |
            |   - REDIRECTION_EO, redirection des sorties erreur et standard.                 |
            |                                             |
            | - e.gauche et e.droite, de type Expression *, représentent une sous-expression gauche |
            |       et une sous-expression droite. Ces deux champs ne sont pas utilisés pour les    |
            |       types VIDE et SIMPLE. Pour les expressions réclamant deux sous-expressions      |
            |       (SEQUENCE, SEQUENCE_ET, SEQUENCE_OU, et PIPE) ces deux champs sont utilisés     |
            |       simultannément.  Pour les autres champs, seule l'expression gauche est        |
            |       utilisée.                                         |
            |                                             |
            | - e.arguments, de type char **, a deux interpretations :                |
            |                                             |
            |      - si le type de la commande est simple, e.arguments pointe sur un tableau à la   |
            |       argv. (e.arguments)[0] est le nom de la commande, (e.arguments)[1] est le         |
            |       premier argument, etc.                                |
            |                                             |
            |      - si le type de la commande est une redirection, (e.arguments)[0] est le nom du  |
            |       fichier vers lequel on redirige.                              |
            `--------------------------------------------------------------------------------------*/

            Expression *e = ExpressionAnalysee;
			prepareExpression(e);
            
	   
            if(exitAsked)
                break;
        }
        else
        {
            /* L'analyse de la ligne de commande a donné une erreur */
            fprintf (stderr,"Expression syntaxiquement incorrecte !\n");
            reset_command_line();
        }
    }
    uninitCommands();
    return 0;
}

void prepareExpression(Expression *e) {
            /*fprintf (stderr,"Expression syntaxiquement correcte : ");
            fprintf (stderr,"[%s]\n", previous_command_line());*/

            char* cmd = previous_command_line();
            if (cmd && strcmp(cmd,""))
                addToHistory(cmd);

            //fprintf(stderr, "%%%% CMD = %s \n",cmd);


            char *nonJobCommand=malloc(sizeof(char)*256);
            fetchFirstArg(cmd, &nonJobCommand);


            //fprintf(stderr, "---- FIRST WORD COMMAND = %s ----\n",nonJobCommand);

            bool isNJC = (strcmp(nonJobCommand, "jobs") == 0 || strcmp(nonJobCommand, "fg") == 0 || strcmp(nonJobCommand, "bg") == 0);
         
            if(! isNJC)
                job_new(cmd);
            lastExitCode = executeExpression(e);
            if(!isBG && !isNJC)
            {
                //fprintf(stderr, "job_WAIT\n");
                lastExitCode = job_wait(job_current()); //, isBG);
            }

            isBG=false;
            finishTime(getpid());

            expression_free(e);
            free(nonJobCommand);

            
            job_check_childs();
            // purge finished jobs
            job_purge(); 
            
            
}

int executeExpression(Expression *e)
{
    if (e->type == VIDE)  // empty expression
    {
    	//printf("VIDE\n");
        printf("\n");
        return 0;
    }
    if (e->type == SIMPLE)
    {
        if(shellParams.verbose)
        {
            printf("il s'agit d'une commande simple dont voici les arguments :\n");
            for(int i=0; e->arguments[i] != NULL; i++)
                printf("[%s]",e->arguments[i]);
            putchar('\n');
        }

        if(strcmp(e->arguments[0], "exit") == 0)
        {
            exitAsked = true;
            return 0;
        }
        else
        {
            char **target = getAlias(e->arguments[0]);
            if(target)   //Globalement, ce bloc remplace la commande actuelle (e->arguments) par la nouvelle avec application de l'alias
            {
                int argsLen = 0;
                for(; e->arguments[argsLen+1]; argsLen++) {} //Trouver le nombre d'arguments sans le nom de la commande

                int targetLen = 0;
                for(; target[targetLen]; targetLen++) {} //Trouver la taille de la commande cible

                char **newArguments = malloc((targetLen+argsLen+1)*sizeof(char*));

                for(int i=0; i<targetLen; i++)
                {
                    int len = strlen(target[i]);
                    newArguments[i] = malloc((len+1)*sizeof(char));
                    strcpy(newArguments[i], target[i]);
                }

                for(int i=0; i<argsLen; i++)
                    newArguments[i+targetLen] = e->arguments[i+1];

                newArguments[targetLen+argsLen] = NULL;

                free(e->arguments[0]);
                free(e->arguments);

                e->arguments = newArguments;
            }

            commandFct fct = getCommand(e->arguments[0]);

            //int retValue;

            if(fct) {
				startTime(getpid());
                return fct(e->arguments); // Internal command executed
                
            }
            else
            {
                char *binfn=externalCommandExists(e->arguments[0]);
                if(binfn != NULL)
                {
                    free(e->arguments[0]);
                    e->arguments[0] = binfn;
                    return execExternalCommand(e->arguments);
                }
                printf("%s: command not found\n", e->arguments[0]);
                return EXIT_FAILURE;
            }
        }
    }
    else if(e->type == SEQUENCE)  // ; expression
    {
        int retValueLeft = executeExpression(e->gauche);
        int retValueRight = executeExpression(e->droite);
        return (retValueLeft || retValueRight);
    }
    else if(e->type == SEQUENCE_ET)  // && expression
    {
        int retValue = executeExpression(e->gauche);
        if(retValue == EXIT_SUCCESS)   // if the return value of the first children is EXIT_SUCCESS execute second expression
        {
            retValue = executeExpression(e->droite);
            return retValue;
        }
        return EXIT_FAILURE;

    }
    else if(e->type == SEQUENCE_OU)
    {
        int retValue = executeExpression(e->gauche);
        if(retValue != EXIT_SUCCESS)   // execute only if first failed
        {
            retValue = executeExpression(e->droite);
            return retValue;
        }
        return EXIT_SUCCESS;
    }
    else if(e->type == BG)
    {
        isBG = true;
        return executeExpression(e->gauche);
    }
    else if(e->type == PIPE)
    {
        int fd[2];
        pipe(fd); //fd[0] pour lire, fd[1] pour écrire

            int savedStdout = dup(1);
            dup2(fd[1], 1);
            executeExpression(e->gauche);
            dup2(savedStdout, 1);
            close(savedStdout);
            close(fd[1]);
        
                int savedStdin = dup(0);
                dup2(fd[0], 0);
                int retValue = executeExpression(e->droite);
                dup2(savedStdin, 0);
                close(savedStdin);
                close(fd[0]);
           
        

        return retValue;
    }
    else if(e->type == REDIRECTION_I)
    {
        int fd = open(e->arguments[0], O_RDONLY);
        int retValue;

        if(fd>=0)
        {
            int savedStdin = dup(0);
            dup2(fd, 0);
            retValue = executeExpression(e->gauche);
            dup2(savedStdin, 0);
            close(savedStdin);
            close(fd);
        }
        else
        {
            fprintf(stderr, "Une erreur est survenue lors de l'ouverture de \"%s\"\n", e->arguments[0]);
            return EXIT_FAILURE;
        }

        return retValue;
    }
    else if(e->type == REDIRECTION_O)
    {
        int fd = open(e->arguments[0], O_WRONLY | O_CREAT | O_TRUNC);
        int retValue;

        if(fd>=0)
        {
            int savedStdout = dup(1);
            dup2(fd, 1);
            retValue = executeExpression(e->gauche);
            dup2(savedStdout, 1);
            close(savedStdout);
            close(fd);
        }
        else
        {
            fprintf(stderr, "Une erreur est survenue lors de l'ouverture de \"%s\"\n", e->arguments[0]);
            return EXIT_FAILURE;
        }

        return retValue;
    }
    else if(e->type == REDIRECTION_A)
    {
        int fd = open(e->arguments[0], O_WRONLY | O_CREAT | O_APPEND);
        int retValue;

        if(fd>=0)
        {
            int savedStdout = dup(1);
            dup2(fd, 1);
            retValue = executeExpression(e->gauche);
            dup2(savedStdout, 1);
            close(savedStdout);
            close(fd);
        }
        else
        {
            fprintf(stderr, "Une erreur est survenue lors de l'ouverture de \"%s\"\n", e->arguments[0]);
            return EXIT_FAILURE;
        }

        return retValue;
    }
    else if(e->type == REDIRECTION_E)
    {
        int fd = open(e->arguments[0], O_WRONLY | O_CREAT | O_TRUNC);
        int retValue;

        if(fd>=0)
        {
            int savedStderr = dup(2);
            dup2(fd, 2);
            retValue = executeExpression(e->gauche);
            dup2(savedStderr, 2);
            close(savedStderr);
            close(fd);
        }
        else
        {
            fprintf(stderr, "Une erreur est survenue lors de l'ouverture de \"%s\"\n", e->arguments[0]);
            return EXIT_FAILURE;
        }

        return retValue;
    }
    else if(e->type == REDIRECTION_EO)
    {
        int fd = open(e->arguments[0], O_WRONLY | O_CREAT | O_TRUNC);
        int retValue;

        if(fd>=0)
        {
            int savedStdout = dup(1);
            int savedStderr = dup(2);
            dup2(fd, 1);
            dup2(fd, 2);
            retValue = executeExpression(e->gauche);
            dup2(savedStderr, 2);
            dup2(savedStdout, 1);
            close(savedStderr);
            close(savedStdout);
            close(fd);
        }
        else
        {
            fprintf(stderr, "Une erreur est survenue lors de l'ouverture de \"%s\"\n", e->arguments[0]);
            return EXIT_FAILURE;
        }

        return retValue;
    }
    else
    {
        fprintf(stderr,"Expression type not implemented (NUMBER %d)\n", e->type);
        return EXIT_FAILURE;
    }
}

bool getHostName(char *buff)
{
    FILE *hostnameFile = fopen("/etc/hostname", "r");

    if(hostnameFile)
    {
        int length = fread(buff, sizeof(char), 256, hostnameFile);

        if(length < 1)
        {
            buff[0] = '\0';
            fclose(hostnameFile);
            return false;
        }
        else
        {
            for(int i=0; i<length; i++)
                if(buff[i] == '\n')
                {
                    buff[i] = '\0';
                    break;
                }
            buff[length] = '\0';
        }

        fclose(hostnameFile);
    }
    else
        return false;

    return true;
}


void fetchFirstArg(char *cmd, char **p_nonJobCommand)
{
    char *nonJobCommand = *(p_nonJobCommand);
    int indexNJC=0, indexCMD=0;
    while(cmd[indexCMD]!='\0' && (cmd[indexCMD]==' ' || cmd[indexCMD]=='\n' ) )
    {
        indexCMD++;
    }
    while(cmd[indexCMD]!='\0' && (cmd[indexCMD]!=' ' || cmd[indexCMD]=='\n')){
        nonJobCommand[indexNJC] = cmd[indexCMD];
        indexNJC++;
        indexCMD++;
    }
    nonJobCommand[indexNJC]= '\0';
}
