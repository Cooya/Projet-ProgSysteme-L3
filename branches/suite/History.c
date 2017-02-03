#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "History.h"
#include "extlib.dll.h"
#include "EnvVars.h"

static char* filename = "/.Shell_history";
static char* path;
static FILE* file;
static int size;
static int currentIndex;

static void openFile() {
	file = fopen(path, "a+");
}

static bool fileExists() { // faux => le fichier a été supprimé
	if (access(path,F_OK))
		return true;
	fclose(file); // pour éviter une fuite de mémoire
	return false;
}

static void setHistorySize() {
	fseek(file, 0, SEEK_SET);
	size = 0;
	char* line = NULL;
	size_t len;
	while (getline(&line, &len, file) != -1) {
		size++;
	}
	if(line)
		free(line);
}
    
void initHistory() {
	char* home = getEnvVar("HOME");
	path = malloc((int) (strlen(home) + strlen(filename))+1);
	path[0] = '\0';
	strcat(path, home);
    strcat(path, filename);
	openFile();
	setHistorySize();
	currentIndex = size;
}

void uninitHistory() {
	fclose(file);
	free(path);
}

void addToHistory(char* cmd) {
	if (!fileExists())
		openFile();
	
	int len=strlen(cmd)+1; // comprend aussi le retour à la ligne
	char element[len+1];
	element[0] = '\0';
	strcat(element, cmd);
	strcat(element, "\n");
	fwrite(&element, sizeof(char), len, file);
	size++;
	currentIndex = size;
}

void printHistory() {
	fseek(file, 0, SEEK_SET); 
	int i = 1;
	char* line = NULL;
	size_t len;
	while (getline(&line, &len, file) != -1) {
		printf("   %d  %s",i , line);
		i++;
	}
	if(line)
		free(line);
}

char* getCommandFromHistory(bool b) {
	if(b == UP) {
		if(currentIndex == 0)
			return NULL;
		currentIndex--;
	}
	else if (b == DOWN) {
		if(currentIndex >= size - 1)
			return NULL;
		currentIndex++;
	}
	fseek(file, 0, SEEK_SET);
	int i = 0;
	char* line = NULL;
	size_t len;
	do {
		if (getline(&line, &len, file) == -1) {
			if(line)
				free(line);
			return NULL;
		}
		i++;
	}
	while(currentIndex >= i);
	return line;
}

void emptyHistory() {
	fclose(file);
	file = fopen(path, "w"); // écrasement
	fclose(file);
	openFile();
	size = 0;
	currentIndex = 0;
}
