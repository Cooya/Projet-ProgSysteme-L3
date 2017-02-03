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
IDS ([-.$/\\ *?A-Za-z0-9]+)
/*ID	([-.$/\\=*?A-Za-z0-9"(.*?)"]+)*/

%%

[ \t]+			_CAT;
^[ \t]*\n		_CAT;
{ID} {
  _CAT;
  strcpy (yylval.identificateur, yytext);
  return IDENTIFICATEUR;
  }
\"{IDS}\"|\'{IDS}\' {
  _CAT;
  int pos = strlen(yytext);
  yytext[pos-1]='\0';
  strcpy (yylval.identificateur, yytext+1);
  return IDENTIFICATEUR;
  }
{ID}={ID} {
  _CAT;
  char *equalPtr = strchr(yytext, '=');
  *equalPtr = '\0';
  int offset = 0;
  yylval.identificateur[offset++] = '=';
  strcpy (yylval.identificateur+offset, yytext);
  offset += equalPtr-yytext;
  yylval.identificateur[offset++] = ' ';
  strcpy (yylval.identificateur+offset, equalPtr+1);
  return IDENTIFICATEUR;
  }
{ID}=\"{IDS}\"|{ID}=\'{IDS}\' {
  _CAT;
  int pos = strlen(yytext);
  yytext[pos-1]='\0';
  char *equalPtr = strchr(yytext, '=');
  *equalPtr = '\0';
  int offset = 0;
  yylval.identificateur[offset++] = '=';
  strcpy (yylval.identificateur+offset, yytext);
  offset += equalPtr-yytext;
  yylval.identificateur[offset++] = ' ';
  strcpy (yylval.identificateur+offset, equalPtr+2);
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
