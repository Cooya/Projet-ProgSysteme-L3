%{
#include "Shell.h"
#include "y.tab.h"

#define MAX_CMD_LINE 1024

static char cmd_line[2][MAX_CMD_LINE];
static unsigned previous_index = 0, current_index = 1;

#define _CAT strcat(cmd_line[current_index], yytext)
#define _EOC \
   do { \
      previous_index = 1 - previous_index; \
      current_index = 1 - current_index; \
      reset_command_line(); \
   } while(0)

char *previous_command_line(void)
{
   return cmd_line[previous_index];
}

void reset_command_line(void)
{
   cmd_line[current_index][0] = '\0';
}

%}

ID	([-.$/\\*?A-Za-z0-9]+)

%%

[ \t]+			_CAT;
^[ \t]*\n		_CAT;
{ID}|\"{ID}\"|\'{ID}\' {
  _CAT;
  strcpy (yylval.identificateur, yytext);
  return IDENTIFICATEUR;
  }
\<			_CAT; return IN;
\>			_CAT; return OUT;
"2>"			_CAT; return ERR;
"&>"			_CAT; return ERR_OUT;
">>"			_CAT; return OUT_APPEND;
"||"			_CAT; return OU;
"&&"			_CAT; return ET;
<<EOF>>			EndOfFile();
\n			_EOC; return yytext[0];
.                       _CAT; return yytext[0];
