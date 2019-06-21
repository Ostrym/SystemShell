#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pwd.h>
#include "commandesInterne.h"

void getLine(char* line_, int* lineSz_)
{
    /*
		 * Lire une ligne (jusqu'au caractère '\n') ; la stocker dans line
		 *
		 * Pour éviter de poser une limite à la taille d'une ligne de commande,
		 * on va lire l'entrée par morceaux de BUF_SZ caractères (fonction fgets) et
		 * on va augmenter dynamiquement la taille de line (fonction realloc)
	*/
    const size_t BUF_SZ = 64;
	char buffer[BUF_SZ];
    bool eol = false;
	while (!eol)	// récupérer toute la ligne en découpant par bouts
	{
		fgets(buffer, BUF_SZ, stdin);
		size_t buf_sz = strlen(buffer); //taille effective
		if (buffer[buf_sz-1] == '\n')
		{
            if(buf_sz > 1 && buffer[buf_sz-2] == '\\')
            {
                buffer[buf_sz-2] = '\0';
                buffer[buf_sz-1] = ' ';
                buf_sz--;
            }
            else
            {
                eol = true;
            }
                

            buffer[buf_sz-1] = '\0';
            buf_sz--;
                
		}
		*lineSz_ += buf_sz;
		line_ = (char*)realloc(line_, ((*lineSz_)+1)*sizeof(char));
		strcat(line_, buffer);
	}
}

void getTokens(char** args_array, char* line, int* ntok_read, int* args_sz, char **ifile, char **ofile, int* fd, int* isPipe, char **nextCommand)
{
    // découper la ligne en tokens (fonction strtok)
	// stocker dans args_array et traiter les redirections
    char* token = strtok(line, " "); // premier token
	


	while (token != NULL)
	{
		// Redirections ?
		if (!strcmp(token,"<") || !strcmp(token,">")|| !strcmp(token,"2>"))
		{
			char redirect[2];
			strcpy(redirect, token);
			token = strtok(NULL, " "); //nom du fichier (token suivant)

			if (!strcmp(redirect, "<"))
			{
				*ifile = token;
				*fd = fileno(stdin);
			}
			else if(!strcmp(redirect, ">"))
			{
				*ofile = token; // redirection de sortie
				*fd = fileno(stdout);
				
			}
			else if(!strcmp(redirect, "2>"))
			{
				*ofile = token; // redirection de sortie
				*fd = fileno(stderr);
			}
		}
		else if(!strcmp(token,"|")) // PIPE ?
		{
			token = strtok(NULL, " "); // nom de la commande suivante
			*nextCommand = token;
		}
		else
		{
			if (*ntok_read >= *args_sz) //pas assez de mémoire allouée pour args_array
			{
				*args_sz = 2*(*args_sz); // doubler la taille de args_array
				args_array = (char **)realloc(args_array, (*args_sz)*sizeof(char*));
			}
			args_array[*ntok_read] = token;
			++(*ntok_read);
		}
		token = strtok(NULL, " "); // token suivant
	}
	// stocker le NULL final dans args_array
	if (*ntok_read >= *args_sz)
		args_array = (char **)realloc(args_array, ((*args_sz)+1)*sizeof(char*));
	args_array[*ntok_read] = NULL; // marqueur pour execv pour savoir où terminer
}

int internalCommands(char** args_array, int nb, char** cwd, int cwdBuffSize)
{
	if(!strcmp(args_array[0], "exit"))
    {
        free(args_array);
		exit(0);
    }
    else if(!strcmp(args_array[0], "cd"))
    {
        if(nb != 2)
        {
            printf(    "Incorrect usage of 'cd':\n"
                   "cd 'newDirectory'\n");
            return 1;
        }
        
        if(chdir(args_array[1]) == 0)
            getcwd(*cwd, cwdBuffSize);
        else
            perror("Error");
        
        return 1;
    }
    else if(!strcmp(args_array[0], "cp"))
    {
        pid_t pidf;
        pidf = fork();
        if(pidf == 0){
            cp(args_array);
        }
        wait(NULL);
        
        return 1;
    }
    else if(!strcmp(args_array[0], "cat"))
    {
        pid_t pidf;
        pidf = fork();
        if(pidf == 0)
            cat(args_array);
        wait(NULL);
        
        return 1;
    }
    
    else if(!strcmp(args_array[0], "ls"))
    {
        pid_t pidf;
        pidf = fork();
        if(pidf == 0){
            ls(args_array, nb);
        }
        wait(NULL); // On attend que le processus fils se finit
        
        
        return 1;
    }
    else if(!strcmp(args_array[0], "find"))
    {
        pid_t pidf;
        pidf = fork();
        if(pidf == 0){
            find(args_array, nb);
        }
        wait(NULL); // On attend que le processus fils se finit
        
        return 1;
    }
    
    else if(!strcmp(args_array[0], "setenv"))
    {
        pid_t pidf;
        pidf = fork();
        if(pidf == 0){
            setMyEnv(args_array, nb);
        }
        wait(NULL); // On attend que le processus fils se finit
        
        return 1;
    }
    
    return 0;
}

int main(int argc, char** argv)
{
	
	const char *invite = "> ";

	const int BUFFSIZE = 64;
    char login[BUFFSIZE];
    char* cwd = (char*)calloc(BUFFSIZE, sizeof(char));
    char host[BUFFSIZE];

    strcpy(login, (*getpwuid(getuid())).pw_name);
    gethostname(host, BUFFSIZE);
    getcwd(cwd, BUFFSIZE);

	while (true)
	{
		// Afficher invite
		printf("%s@%s:%s%s", login, host, cwd, invite);
        
		char* line = (char *)calloc(1, sizeof(char)); // calloc alloue une case mémoire contenant '\0'
		int line_sz = 0; // taille actuelle de line (sans le '\0' final)
		
        getLine(line, &line_sz); // Récupère la ligne
		if(strlen(line)==0)
			continue;

        int ntok_read = 0, args_sz = 10;
		char** args_array = (char **)calloc(10, sizeof(char*));
		char *ifile = NULL, *ofile = NULL; // fichiers de redirection (entrée, sortie)
		int fd = 0;

		int isPipe = 0;
		char *nextCommand = NULL;
		
		getTokens(args_array, line, &ntok_read, &args_sz, &ifile, &ofile, &fd, &isPipe, &nextCommand); // Sépare la ligne en tokens

		pid_t pid = fork();
		if (pid == 0)
		{
			// Child: prepare redirections, execute command
			int i_desc = -1, o_desc = -1;
			if (ifile)
			{
				i_desc = open(ifile, O_RDONLY);
				dup2(i_desc, fd); //or STDIN_FILENO ...
			}
			if (ofile)
			{
				o_desc = open(ofile, O_CREAT | O_WRONLY, 0644);
				dup2(o_desc, fd); //or STDOUT_FILENO ...
			}
			// Original files can be closed: they just have been duplicated
			if (ifile)
				close(i_desc);
			if (ofile)
				close(o_desc);

			// Si pipe
			if(nextCommand)
			{
				/* create the pipe */
				int  pfd[2];
				if (pipe(pfd) == -1)
				{
					printf("pipe failed\n");
					return 1;
				}
				int pid2 = fork();
				int BUFSIZE = 16;
				// ENFANT
				if(pid2 == 0)
				{
					close(pfd[1]); // CLOSE WRITE SIDE
					dup2(pfd[0], 0);
					close(pfd[0]);
					// READ
					execlp(nextCommand, nextCommand, (char*)0);
				
					exit(0);
				}
				else // PARENT
				{
					close(pfd[0]); /* close read side */
					dup2(pfd[1], 1);
					close(pfd[1]);
					/* send some data into the pipe */
					execlp(args_array[0], args_array[0], (char*)0);
					exit(0);
				}
				
			}

			/*** The command is executed here ***/
			if(!internalCommands(args_array, ntok_read, &cwd, BUFFSIZE))
			{
				execv(args_array[0], args_array);
				/***                              ***/
				// If the program arrives here, execvp failed
				fprintf(stderr, "Error while launching %s\n", args_array[0]);
				exit(EXIT_FAILURE);
			}
			exit(0);
		}
		else if (pid != 0)
		{
			
			// Parent: wait for child to finish
			int status; //optional; give infos on child termination
			waitpid(pid, &status, 0); //or just wait(&status), or wait(NULL)...
			if(!strcmp(args_array[0],"exit"))
			{
				exit(0);
			}
		}

		// Memory cleanup before next command
		free(args_array);
		free(line);

    }
}