/* Construction des arbres représentant des commandes */

#include "Shell.h"
#include "Commands.h"
#include "Aliases.h"
#include "EnvVars.h"
#include <stdbool.h>

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

    l = Liste + LongueurListe (Liste);
    *l = (char *) (malloc (1+strlen (Arg)));
    strcpy (*l++, Arg);
    *l = NULL;
    return Liste;
} /* AjouterArg */



/*
 * Fonction appelée lorsque l'utilisateur tape "".
 */
void EndOfFile (void)
{
    exit (0);
} /* EndOfFile */

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


int
main (int argc, char **argv, char **envp)
{
	ShellParams shellParams;
	shellParams.verbose = false;
	
    storeEnvVars(envp);

    char hostName[256]; // Longueur maximale pour un nom d'ordinateur
    if(!getHostName(hostName))
        fprintf(stderr, "Error: Hostname not found.\n");

    char *logName = getEnvVar("LOGNAME");
    char *pwd = getEnvVar("PWD");

    initCommands(&shellParams);

    bool exitAsked = false;
    while (1)
    {
        printf("%s@%s:%s# ", logName, hostName, pwd);
        fflush(stdout);

        if (yyparse () == 0)
        {
            /*--------------------------------------------------------------------------------------.
            | L'analyse de la ligne de commande est effectuée sans erreur.  La variable globale     |
            |       ExpressionAnalysee pointe sur un arbre représentant l'expression.  Le type      |
            |       "Expression" de l'arbre est décrit dans le fichier Shell.h. Il contient 4       |
            |       champs. Si e est du type Expression :					      |
            | 										      |
            | - e.type est un type d'expression, contenant une valeur définie par énumération dans  |
            |   Shell.h. Cette valeur peut être :					      	      |
            | 										      |
            |   - VIDE, commande vide								      |
            |   - SIMPLE, commande simple et ses arguments					      |
            |   - SEQUENCE, séquence (;) d'instructions					      |
            |   - SEQUENCE_ET, séquence conditionnelle (&&) d'instructions			      |
            |   - SEQUENCE_OU, séquence conditionnelle (||) d'instructions			      |
            |   - BG, tâche en arrière plan (&)						      |
            |   - PIPE, pipe (|).								      |
            |   - REDIRECTION_I, redirection de l'entrée (<)					      |
            |   - REDIRECTION_O, redirection de la sortie (>)					      |
            |   - REDIRECTION_A, redirection de la sortie en mode APPEND (>>).		      |
            |   - REDIRECTION_E, redirection de la sortie erreur,  	   			      |
            |   - REDIRECTION_EO, redirection des sorties erreur et standard.			      |
            | 										      |
            | - e.gauche et e.droite, de type Expression *, représentent une sous-expression gauche |
            |       et une sous-expression droite. Ces deux champs ne sont pas utilisés pour les    |
            |       types VIDE et SIMPLE. Pour les expressions réclamant deux sous-expressions      |
            |       (SEQUENCE, SEQUENCE_ET, SEQUENCE_OU, et PIPE) ces deux champs sont utilisés     |
            |       simultannément.  Pour les autres champs, seule l'expression gauche est	      |
            |       utilisée.									      |
            | 										      |
            | - e.arguments, de type char **, a deux interpretations :			      |
            | 										      |
            |      - si le type de la commande est simple, e.arguments pointe sur un tableau à la   |
            |       argv. (e.arguments)[0] est le nom de la commande, (e.arguments)[1] est le	      |
            |       premier argument, etc.							      |
            | 										      |
            |      - si le type de la commande est une redirection, (e.arguments)[0] est le nom du  |
            |       fichier vers lequel on redirige.						      |
            `--------------------------------------------------------------------------------------*/

            Expression *e = ExpressionAnalysee;

            /*fprintf (stderr,"Expression syntaxiquement correcte : ");
            fprintf (stderr,"[%s]\n", previous_command_line());*/

            if (e->type == SIMPLE)
            {
				if(shellParams.verbose) {
					printf("il s'agit d'une commande simple dont voici les arguments :\n");
					for(int i=0; e->arguments[i] != NULL;i++)
						printf("[%s]",e->arguments[i]);
					putchar('\n');
				}

                if(strcmp(e->arguments[0], "exit") == 0)
                    exitAsked = true;
                else
                {
					char **target = getAlias(e->arguments[0]);
					if(target) { //Globalement, ce bloc remplace la commande actuelle (e->arguments) par la nouvelle avec application de l'alias
                        int argsLen = 0;
                        for(; e->arguments[argsLen+1]; argsLen++) {} //Trouver le nombre d'arguments sans le nom de la commande

                        int targetLen = 0;
                        for(; target[targetLen]; targetLen++) {} //Trouver la taille de la commande cible

                        char **newArguments = malloc((targetLen+argsLen+1)*sizeof(char*));

                        for(int i=0; i<targetLen; i++) {
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

                    int retValue;

                    if(fct)
                        retValue = fct(e->arguments); // Internal command executed
                    else if(externalCommandExists(e->arguments[0]))
                        retValue = execExternalCommand(e->arguments); // External command executed
                    else
                        printf("%s: command not found\n", e->arguments[0]);
                }
            }
            expression_free(e);

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

bool getHostName(char *buff) {
    FILE *hostnameFile = fopen("/etc/hostname", "r");

    if(hostnameFile)
    {
        int length = fread(buff, sizeof(char), 256, hostnameFile);

        if(length < 1) {
            buff[0] = '\0';
            fclose(hostnameFile);
            return false;
        }
        else {
            for(int i=0; i<length; i++)
                if(buff[i] == '\n') {
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
