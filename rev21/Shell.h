#ifndef ANALYSE
#define ANALYSE

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define NB_ARGS 50
#define TAILLE_ID 500

typedef enum expr_t {
  VIDE,             // Commande vide
  SIMPLE,           // Commande simple
  SEQUENCE,         // S�quence (;)
  SEQUENCE_ET,      // S�quence conditionnelle (&&)
  SEQUENCE_OU,      // S�quence conditionnelle (||)
  BG,             	// Tache en arriere plan
  PIPE,             // Pipe
  REDIRECTION_I,    // Redirection entree
  REDIRECTION_O,    // Redirection sortie standard
  REDIRECTION_A,    // Redirection sortie standard, mode append
  REDIRECTION_E,    // Redirection sortie erreur
  REDIRECTION_EO,   // Redirection sorties erreur et standard
} expr_t;

typedef struct Expression {
  expr_t type;
  struct Expression *gauche;
  struct Expression *droite;
  char   **arguments;
} Expression;

int yyparse(void);
extern char *previous_command_line(void);
extern void reset_command_line(void);
Expression *ConstruireNoeud (expr_t, Expression *, Expression *, char **);
char **AjouterArg (char **, char *);
char **InitialiserListeArguments (void);
int LongueurListe(char **);
void EndOfFile(void);
int executeExpression(Expression *e);

void yyerror (char *s);
Expression *ExpressionAnalysee;

bool getHostName(char *buff);

typedef struct _ShellParams {
  bool verbose;
} ShellParams;

#endif /* ANALYSE */
