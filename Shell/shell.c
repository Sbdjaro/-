#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

void logCommands(char ***tokens)
{
    printf("executing %s with arguments: ", tokens[0][0]);
        {
            int i = 0;
            while (tokens[0][i])
            {
                printf("%s ", tokens[0][i]);
                i++;
            }
            printf("\n");
        }
}

void handleError(const char *errorMsg)
{
    fprintf(stderr, "%s\n", errorMsg);
    exit(EXIT_FAILURE);
}

void checkRedirection(char** s, int *x, int *y)
{
	for (int j = 0; s[j]; j++)
	{
		for (int i = 0; i < 3; i++)
		{
			if (s[j][0] == '>')
			{
				*x = j;
			}

			if (s[j][0] == '<')
			{
				*y = j;
			}
		}
	}
	return;
}

typedef struct Plus 
{
	int count;
	int last;
	int pos[100];
} Plus;

Plus checkPulse(char** s)
{
	int size = 0;
	Plus plus;
	plus.count = 0;
	plus.last = 0;
	for (int i = 0; i < 100; i++)
	{
		plus.pos[i] = -1;
	}
	int j;
    for (j = 0; s[j]; j++)
	{
		if (s[j][0] == '+')
		{
			plus.pos[++size] = j;
			plus.last = j;
		}
	}
	plus.pos[size + 1] = j;
	plus.count = size;
    return plus;
}

char* processInput(int *status)
{
	int max_size = 128;
	int size = 0;
	char *line = (char*)calloc(max_size, sizeof(char));
	
	if (!line)
	{
        handleError("allocation failure while processing user input\n");
	}
	while (1)
	{
		char c = getchar();

		if (c == EOF || c == '\n')
		{
			line[size] = '\0';

			if (line[size - 1] == '&')
			{
				line[size - 1] = '\0';
				*status = 1;
			}
			return line;
		}
		else
		{
			line[size] = c;
		}
		size++;

		if (size >= max_size)
		{
			max_size *= 2;
			line = (char*)realloc(line, max_size * sizeof(char));
			if (!line)
			{
                free(line);
				handleError("reallocation failure while processing user input\n");
			}	
		}
	}
	if (line[size - 1] == '&')
	{
		line[size - 1] = '\0';
		*status = 1;
	}
	return line;
}

char** splitPipes(char *input, int *n)
{
	int max_size = 128;
	int size = 0;

	char **commands = (char**)calloc(max_size, sizeof(char*));

	if (!commands)
	{
		handleError("failure while parcing user input");
	}
	
	char *token;
	
	token = strtok(input, "|");
	
	while (token)
	{
		commands[size] = token;
		size++;
	
		if (size >= max_size)
		{
			max_size *= 2;
			commands = (char**)realloc(commands, max_size * sizeof(char*));
			if (!commands)
			{
				handleError("reallocation failure while parcing user input\n");
			}
		}
		token = strtok(NULL, "|");
	}

	*n = size;
	commands[size] = NULL;
	return commands;
}

char*** create3DArray(int max_size, int com_length)
{	
	char ***tokens = (char***)calloc(max_size, sizeof(char**));

	if (!tokens)
	{
		handleError("allocation failure while parcing single command\n");
	}

	for (int i = 0; i < max_size; i++)
	{
		tokens[i] = (char**)calloc(com_length, sizeof(char*));

		if (!tokens[i])
		{
			handleError("allocation failure while parcing single command\n");
		}
	}
	return tokens;
}

char*** reallocate3DArray(char ***tokens, int new_max_size, int new_com_length)
{
	tokens = (char***)realloc(tokens, new_max_size * sizeof(char**));
	if (!tokens)
	{
        handleError("reallocation failure\n");
	}

	for (int i = 0; i < new_max_size; i++)
	{
		tokens[i] = (char**)realloc(tokens[i], new_com_length * sizeof(char*));

		if (!tokens[i])
		{
			handleError("reallocation failure\n");
		}
	}
	return tokens;
}

void free3DArray(char ***tokens, int max_size)
{
	for (int i = 0; i < max_size; i++)
	{
		free(tokens[i]);
	}
	free(tokens);
}

char*** parseCommands(char **commands)
{
	int max_size = 128;
	int com_length = 128;
	int size = 0;
	
	char ***tokens = create3DArray(max_size, com_length);

	while (commands[size])
	{
		char *token;

		token = strtok(commands[size], " ");
		int j = 0;
		while (token)
		{
			tokens[size][j] = token;
			j++;
			
			if (j >= com_length)
			{
				com_length *= 2;
				tokens = reallocate3DArray(tokens, max_size, com_length);
			}

			token = strtok(NULL, " ");	
		}
		size++;
	}
	tokens[size] = NULL;
	return tokens;
}

int handlePlus(char ***tokens, int* fs, int dsc, int *write_to)
{
	Plus plus = checkPulse(tokens[0]);
    	int size = plus.count + 1;
	int** fds = (int**)calloc(size, sizeof(int*));
	
	for (int i = 0; i < size; i++)
	{
		fds[i] = (int*)calloc(2, sizeof(int));
	}

	for (int i = 0; i < size; i++)
	{
		if (pipe(fds[i]) < 0) handleError("pipe creating failed\n");
	}
	int** fds2 = (int**)calloc(size, sizeof(int*));

	for (int i = 0; i < size; i++)
	{
		fds2[i] = (int*)calloc(2, sizeof(int));
	}

	for (int i = 0; i < size; i++)
	{
		if (pipe(fds2[i]) < 0) handleError("pipe creating failed\n");
	}
	int x = -1, y = -1;
	checkRedirection(tokens[0], &x, &y);
	
	if (y != -1) 
	{
		if (dsc != -1)
		{
			handleError("invalid command\n");
		}

		int file = open(tokens[0][y + 1], O_RDONLY | O_CREAT, 0777);
		if (file == -1)
		{
			handleError("fd openning failure\n");
		}

		dsc = file;
	}
	
	if (x != -1)
	{
		int file;
		if (strcmp(tokens[0][x], ">>") == 0)
		{
			file = open(tokens[0][x + 1], O_WRONLY | O_CREAT | O_APPEND, 0777);
		}
		else
		{
			file = open(tokens[0][x + 1], O_WRONLY | O_CREAT | O_TRUNC, 0777);
		}

		if (file == -1)
		{
			handleError("fd openning failure\n");
		}
		*write_to = file;
	}
	else
		*write_to = 1;


	int rd;
	char buf[105];
	//if (dsc==-1)
		//dsc=0; //Если убрать комментарий и изменять dsc, то придется каждый раз при программе не требующей ввода нажимать конец ввода, иначе, если оставить комментарий, невозможно выполнять процессы, требующие ввода.
	
	while ((rd = read(dsc, buf, 100)) > 0)
		for (int i = 0; i < size; i++) {
			write(fds2[i][1], buf, rd);
		}
	if (dsc!=0)
		close(dsc);

	for (int i = 0; i < size; i++)
	{
		pid_t pid = fork();

		if (pid < 0)
		{
			handleError("fork failure\n");
		}

		if (pid == 0)
		{	

			dup2(fds[i][1], 1);
			dup2(fds2[i][0], 0);
			for (int k = 0; k < size; k++)
			{
				close(fds[k][0]);
				close(fds[k][1]);
				close(fds2[k][0]);
				close(fds2[k][1]);
				free(fds[k]);
				free(fds2[k]);
			}
			free(fds);
			free(fds2);
			char **commands = (char**)calloc(100, sizeof(char*)); //Не получилось освободить эту память, так как она попадает на экзек
			int id = 0;
			for (int j = plus.pos[i] + 1; j < plus.pos[i + 1]; j++)
			{
				commands[id++] = strdup(tokens[0][j]);
			}
			commands[id] = NULL;
			tokens[0] = commands;
			return 1;
		}
	}

	for (int i = 0; i < size; i++)
	{
		close(fds[i][1]);
		close(fds2[i][1]);
		close(fds2[i][0]);
		free(fds2[i]);
	}

	for (int i = 0; i < size; i++)
	{
		wait(NULL);
	}

	int fd[2];

	if (pipe(fd) < 0) handleError("pipe creation failed");
	

	for (int i = 0; i < size; i++)
	{
		while ((rd = read(fds[i][0], buf, 100)) > 0)
		{
			write(fd[1], buf, rd);
		}
		close(fds[i][0]);
		free(fds[i]);
	}
	close(fd[1]);
	free(fds);
	free(fds2);
	//free(tokens[0]);
	*fs = fd[0];
	return 2;
}

int executePipe(char ***tokens, int *status, int input_fd)
{
	if (*status == 1 && input_fd == -1)
	{
		pid_t pid = fork();
		
        if (pid < 0)
        {
            fprintf(stderr, "fork failure\n");
            exit(1);
        }

		if (pid != 0)
		{
			return 0;	
		}
	}

	if (strcmp(tokens[0][0], "exit") == 0)
	{
        	waitpid(0, NULL, WNOHANG);
		return 2;
	}

	if (tokens[1] == NULL)
	{

		int fs;
		int write_to;
		int exitStatus = handlePlus(tokens, &fs, input_fd, &write_to);
		if (exitStatus!=2)
		{
			int x = -1, y = -1;
			checkRedirection(tokens[0], &x, &y);
			if (x != -1)
			{
				do
				{
					tokens[0][x] = tokens[0][x + 2];
					x++;
				} while (tokens[0][x - 1] != NULL);

			}

			checkRedirection(tokens[0], &x, &y);

			if (y != -1)
			{
				do
				{
					tokens[0][y] = tokens[0][y + 2];
					y++;
				} while (tokens[0][y - 1] != NULL);
			}
			execvp(tokens[0][0], tokens[0]);
		}

		
		int rd;
		char buf[105];
		while ((rd = read(fs, buf, 100)) > 0)
		{
			write(write_to, buf, rd);
		}
		close(fs);

		if (*status == 1)
		{
			return 2;
		}
	}
	else
	{
		
		int fs;
		int write_to;
		int exitStatus = handlePlus(tokens, &fs,input_fd, &write_to);
		if (exitStatus != 2) 
		{
			int x = -1, y = -1;
			checkRedirection(tokens[0], &x, &y);

			if (x != -1)
			{
				handleError("invalid command\n");
			}

			if (y != -1)
			{
				int file = open(tokens[0][y + 1], O_RDONLY | O_CREAT, 0777);
				if (file == -1)
				{
					handleError("file opnenig failure\n");
				}

				dup2(file, STDIN_FILENO);
				close(file);

				do
				{
					tokens[0][y] = tokens[0][y + 2];
					y++;
				} while (tokens[0][y - 1] != NULL);
			}
					
			execvp(tokens[0][0], tokens[0]);
		}

		executePipe(++tokens, status, fs);
	}
    return 0;
}

void loop()
{
	int max_size = 128;
	int status;
	char cwd[max_size];
	char *userInput;
	char **commands;
	char ***tokens;
	int numPipes;
	while (1)
	{
		status = 0;
		if (getcwd(cwd, sizeof(cwd)) == NULL)
		{
			handleError("wtf\n");
		}
		printf("%s> ", cwd);
		userInput = processInput(&status);
		commands = splitPipes(userInput, &numPipes);
		tokens = parseCommands(commands);
		int t = executePipe(tokens, &status, -1);
        	waitpid(0, NULL, WNOHANG);
		free(userInput);
		free(commands);
		free3DArray(tokens, 128);
		if (t==2)
			break;
	}
}

int main()
{
	loop();
	return 0;
}