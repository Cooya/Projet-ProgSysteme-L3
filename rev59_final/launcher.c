#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main() {
	
	
	int fd[2];
	
	pipe(fd);
		pid_t pid = fork();

	if(pid>0)
	{
		dup2(fd[0],0);
		close(fd[1]);
		execl("./Shell", "./Shell");
		return -1;
	}
	else
	{
		fprintf(stderr, "PROGRAMME DE TEST : Lancement de la batterie de test :\n\n");
		dup2(fd[1],1);
		close(fd[0]);
		sleep(2);
		printf("echo 'Test de la commande printenv :'\n printenv\n");
		sleep(2);
		printf("echo 'Test de la commande printenv :'\n printenv\n");
		sleep(2);
		printf("echo 'Test de la commande printenv :'\n printenv\n");
		sleep(2);
		printf("echo 'Test de la commande printenv :'\n printenv\n");
		sleep(2);
		printf("echo 'Test de la commande printenv :'\n printenv\n");
		sleep(2);
		printf("echo 'Test de la commande printenv :'\n printenv\n");
		sleep(2);
		printf("echo 'Test de la commande printenv :'\n printenv\n");
		sleep(2);
		
		
		printf("exit\n");
		wait(0);

	}
	return 0;
}

