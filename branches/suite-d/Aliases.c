#include "Aliases.h"
#include "extlib.dll.h"

typedef struct _Alias {
    char *alias;
    char **target;
} Alias;

Dll aliases = NULL;


Alias createAlias(char *alias, char **target) {
	Alias a;
	int len;

	len = strlen(alias);
    a.alias = malloc((len+1)*sizeof(char));
    strcpy(a.alias, alias);

    int targetLen = 0;
    for(; target[targetLen]; targetLen++) {}

    a.target = malloc((targetLen+1)*sizeof(char*));
    for(int i=0; i<targetLen; i++) {
        len = strlen(target[i]);
        a.target[i] = malloc((len+1)*sizeof(char));
        strcpy(a.target[i], target[i]);
    }
    a.target[targetLen] = NULL;

    return a;
}

void deleteAlias(Alias a) {
	free(a.alias);

	for(int i=0; a.target[i]; i++)
        free(a.target[i]);
    free(a.target);
}


void initAliases() {
    aliases = dllNew(sizeof(Alias));
}

void uninitAliases() {
	DllNode node = dllGetFront(aliases);

    while(node) {
        Alias *a = dllGetData(node);
        deleteAlias(*a);
        node = dllGetNext(node);
    }

    dllDel(aliases);
}

void addAlias(char *alias, char **target) {
    removeAlias(alias);

    Alias a = createAlias(alias, target);

    dllPushBack(aliases, &a);
}

char **getAlias(char *alias) {
    DllNode node = dllGetFront(aliases);

    while(node) {
        Alias *a = dllGetData(node);
        if(strcmp(a->alias, alias) == 0)
            return a->target;

        node = dllGetNext(node);
    }

    return NULL;
}

void removeAlias(char *alias) {
    DllNode node = dllGetFront(aliases);

    while(node) {
        Alias *a = dllGetData(node);
        if(strcmp(a->alias, alias) == 0) {
            deleteAlias(*a);
            dllPopNode(aliases, node);
            break;
        }

        node = dllGetNext(node);
    }
}
