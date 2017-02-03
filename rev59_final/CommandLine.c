#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include "CommandLine.h"
#include "History.h"

static struct CommandLine {
	int fd[2];
	char c;
	char cmd[MAX_LENGTH];
	char cmd_saved[MAX_LENGTH];
	bool cmd_isSaved;
	char spe[3];
	int i_cmd;
	int i_spe;
	int cursor;
	int cursor_saved;
} CL;
extern FILE* yyin;

static void initArray(char* array, int arraySize, char val) {
	for(int i=0;i<arraySize;++i)
		array[i] = val;
}

static void insertCharIntoCmd(int pos, char elt) {
	for(int i=CL.i_cmd;i>pos;--i)
		if(i != 0)
			CL.cmd[i] = CL.cmd[i-1];
	CL.cmd[pos] = elt;
	CL.i_cmd++;
}

static void deleteCharIntoCmd(int pos) {
	for(int i=pos;i<CL.i_cmd;i++)
		CL.cmd[i] = CL.cmd[i+1];
	CL.cmd[CL.i_cmd--] = '\0';
} 

static void initCmd() {
	initArray(CL.cmd, MAX_LENGTH, '\0');
	CL.i_cmd = 0;
}

static void initSpe() {
	initArray(CL.spe, 3, '\0');
	CL.i_spe = 0;
}

static void print(char c) {
	printf("%c", c);
	CL.cursor++;
}

static void copyStringToCmd(char* str) {
	initCmd();
	int len = strlen(str);
	for(int i=0;i<len;++i)
		CL.cmd[i] = str[i];
	CL.i_cmd=len;
}

static void printCmd() {
	for(int i=0;i<CL.i_cmd;++i)
		print(CL.cmd[i]);
}

static void cToSpe() {
	if (CL.i_spe<2)
		CL.spe[CL.i_spe++] = CL.c;
	else
		CL.spe[CL.i_spe] = CL.c;
}

static void moveCursorLeft() {
	printf("%c%c%c",27,91,68);
	CL.cursor--;
}

static void moveCursorRight() {
	printf("%c%c%c",27,91,67);
	CL.cursor++;
}

static void moveCursorToPosition(int pos) {
	while(CL.cursor>pos)
		moveCursorLeft();
	while(CL.cursor<pos)
		moveCursorRight();
}

static void erase() {
	while(CL.cursor<CL.i_cmd)
		print(CL.cmd[CL.cursor]);
}

static void emptyCommandLine() {
	moveCursorToPosition(0);
	while(CL.cursor<CL.i_cmd)
		print(' ');
	moveCursorToPosition(0);
	initCmd();
}

static void saveCommandLine() {
	for(int i=0;i<CL.i_cmd;++i)
		CL.cmd_saved[i] = CL.cmd[i];
	CL.cmd_saved[CL.i_cmd] = '\0';
}

static int mygetch()
{
    struct termios oldt, newt;
    int ch;
 
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

static void addCommandLineFromHistory(bool b) {
	bool needFree = true;
	if(!CL.cmd_isSaved) {
		saveCommandLine();
		CL.cmd_isSaved = true;
	}
	char* cmd;
	if (b == UP && (cmd = getCommandFromHistory(UP)) == NULL)
		return;
	else if (b == DOWN && (cmd = getCommandFromHistory(DOWN)) == NULL) {
		cmd = CL.cmd_saved;
		CL.cmd_isSaved = false; // si on revient en bas, on peut remodifier la première ligne de commande
		needFree = false;
	}
	emptyCommandLine();
	copyStringToCmd(cmd);
	if (CL.cmd[CL.i_cmd-1] == '\n') // les lignes de l'historique contiennent un \n
		CL.cmd[--CL.i_cmd] = '\0';
	printCmd();
	if(needFree)
		free(cmd);
}

static void initCommandLine()
{
	pipe(CL.fd);
	yyin = fdopen(CL.fd[0],"r");
	CL.c = '\0';
	initCmd();
	initSpe();
	CL.cursor = 0;
	initArray(CL.cmd_saved, MAX_LENGTH, '\0');
	CL.cmd_isSaved = false;
}

bool getCommandLine() {
	initCommandLine();
	while((CL.c = mygetch()) != 10) { // entrée
		if(CL.c == 4) // ctrl-d
			exit(EXIT_SUCCESS);
		if(CL.c == 126) { // suppr
			if(CL.i_cmd != 0) { // rien à supprimer
				deleteCharIntoCmd(CL.cursor);
				CL.cursor_saved = CL.cursor;
				erase();
				print(' ');
				moveCursorToPosition(CL.cursor_saved);
			}	
		}
		else if(CL.c == 127) { // backspace
			if(CL.cursor!=0) {
				deleteCharIntoCmd(CL.cursor-1);
				moveCursorLeft();
				CL.cursor_saved = CL.cursor;
				erase();
				print(' ');
				moveCursorToPosition(CL.cursor_saved);
			}
		}
		else if(CL.spe[CL.i_spe-1] == 91 && CL.spe[CL.i_spe-2] == 27) { // flèches
			if(CL.c == 68) { // gauche
				cToSpe();
				if (CL.cursor>0)
					moveCursorLeft();
			}
			else if(CL.c == 67) { // droite
				cToSpe();
				if(CL.cursor<CL.i_cmd)
					moveCursorRight();
			}
			else if(CL.c == 65) { // haut
				addCommandLineFromHistory(UP);
			}
			else if(CL.c == 66) { // bas
				addCommandLineFromHistory(DOWN);
			}
			initSpe();
		}
		else if(CL.c == 27 || (CL.spe[CL.i_spe-1] == 27 && CL.c == 91)) // 1 caractère spécial ou 2 caractères spéciaux
			cToSpe();
		else if(CL.c>31) {
			if(CL.cursor<CL.i_cmd) {
				insertCharIntoCmd(CL.cursor, CL.c);
				CL.cursor_saved = CL.cursor; // on sauvegarde la position du curseur car il va avancer pour écraser ce qui suit
				erase();
				for(int i=CL.i_cmd;i>CL.cursor_saved+1;--i) // on remet le curseur là où il était
					moveCursorLeft();
			}
			else {
				print(CL.c);
				CL.cmd[CL.i_cmd++] = CL.c;
			}
		}
	}
	printf("\n");
	
	if(CL.i_cmd==0) {
		close(CL.fd[0]);
		close(CL.fd[1]);
		return false;
	}
	write(CL.fd[1],CL.cmd,CL.i_cmd);
	write(CL.fd[1],"\n",1);
	close(CL.fd[1]);
	return true;
}

void closeInput() {
	close(CL.fd[0]);
	fclose(yyin);
}
