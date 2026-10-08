#include "shell_lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>

void shell_printprompt();
void shell_gethome();
void shell_add_history(char *input);
void shell_tokenise(char *input, char **tokens);
void shell_commands(char **tokens);
void shell_restore_path(const char *original_path);

void cmd_history_last();
void cmd_history_positive(int num);
void cmd_history_negative(int num);
void save_history();
void load_history();

void cmd_cd(char **tokens);
void cmd_help();
void cmd_setpath(char **tokens);
void cmd_getpath(char **tokens);
void cmd_history();
void cmd_external_commands(char **tokens);
void cmd_history_commands(char *input, char **tokens);

void cmd_alias(char **tokens);
void cmd_unalias(char **tokens);
void save_aliases();
void load_aliases();
void unalias(char *tokens);

int main()
{

	alias_number = 0;
	aliascommands_number = 0;
	// gets the starting path I think
	const char *name = "PATH";
	const char *original_path = getenv(name);

	int length;
	char input[513] = {0};
	char *tokens[100] = {NULL}; // 100 is random but high enough so shell shouldnt ever fail for long inputs

	shell_gethome();
	load_history();
	load_aliases();

	while (1)
	{

		shell_printprompt();
		fflush(stdout);
		if ((fgets(input, 513, stdin)) == NULL)
		{
			printf("\n");
			break;
		}
		length = strlen(input);

		if (input[length - 1] != '\n')
		{
			printf("Input too long 512byte limit\n");
			length = strlen(input);
			while (input[length - 1] != '\n')
			{
				fgets(input, 513, stdin);
				length = strlen(input);
			}
			continue;
		}

		char *original_input = strdup(input);
		if (input[0] == '!')
		{
			cmd_history_commands(input, tokens);
			continue;
		}

		length = strlen(input);
		if (input[length - 1] == '\n')
		{
			input[length - 1] = '\0';
		}

		shell_tokenise(input, tokens);
		if (!tokens[0])
		{
			continue;
		} // if input was empty go back to start of loop

		if (!strcmp(tokens[0], "exit"))
		{
			break;
		}

		shell_add_history(original_input);
		shell_commands(tokens);
	}

	shell_restore_path(original_path);
	shell_gethome();
	save_history();
	save_aliases();

	return 0;
}

// tokens is a char** so it doesnt just make changes to the local version
void shell_tokenise(char *input, char **tokens)
{

	int i = 0;
	char *token = strtok(input, " \t\n><&;|");

	while (token != NULL && i < 99)
	{

		tokens[i] = token;
		token = strtok(NULL, " \t\n><&;|");
		i++;
	}
	tokens[i] = NULL; // NULL being used to say there are no more tokens in the input
}

// finds the correct command and runs the releated function
void shell_commands(char **tokens)
{

	alias_found = 1;
	while (alias_found)
	{

		if (alias_limit == 4)
		{
			alias_limit = 0;
			printf("Alias Limit: You may only chain 3 or less aliases together\n");
			return;
		}

		alias_found = 0;

		for (int i = 0; i < alias_number; i++)
		{

			if (!strcmp(tokens[0], aliases[i]))
			{

				alias_found = 1;
				alias_limit++;
				int size = 0;

				for (int j = 0; alias_commands[i][j] != NULL; j++)
				{

					size++;
				}

				// tokens size e.g. if a is alias, a b c would be token size of 2
				int tokens_size = 0;
				for (int i = 1; tokens[i] != NULL; i++)
				{
					tokens_size++;
				}

				for (int i = 1; i < tokens_size; i++)
				{

					tokens[size] = malloc(strlen(tokens[i]) + 1);
					strcpy(tokens[size], tokens[i]);
					size++;
				}

				int k = 0;
				for (int j = 0; j < size; j++)
				{

					tokens[k] = malloc(strlen(alias_commands[i][j]) + 1);
					strcpy(tokens[k], alias_commands[i][j]);
					k++;
				}

				tokens[size + tokens_size] = NULL; // set last bit of tokens to null
				break;
			}
		}
	}

	alias_limit = 0;

	if (tokens[0][0] == '!')
	{

		char *phrase = tokens[0];
		history_alias = 1;
		cmd_history_commands(phrase, tokens);
		return;
	}

	if (!strcmp(tokens[0], "cd"))
	{

		cmd_cd(tokens);
	}
	else if (!strcmp(tokens[0], "help"))
	{

		cmd_help();
	}
	else if (!strcmp(tokens[0], "getpath"))
	{

		cmd_getpath(tokens);
	}
	else if (!strcmp(tokens[0], "setpath"))
	{

		cmd_setpath(tokens);
	}
	else if (!strcmp(tokens[0], "history"))
	{

		if (tokens[1] != NULL)
		{
			printf("Cannot have parameters when calling history\n");
			return;
		}
		cmd_history();
	}
	else if (!strcmp(tokens[0], "alias"))
	{

		if (tokens[1] == NULL)
		{
			if (aliases[0] == NULL)
			{
				printf("No aliases set\n");
				return;
			}
			else
			{
				for (int i = 0; i < alias_number; i++)
				{

					printf("%s ", aliases[i]);

					for (int j = 0; alias_commands[i][j] != NULL; j++)
					{
						printf("%s ", alias_commands[i][j]);
					}
					printf("\n");
				}
			}
		}
		else if (tokens[2] == NULL)
		{
			printf("Too little arguments\n");
		}

		else if (tokens[2] != NULL)
		{
			cmd_alias(tokens);
		}
	}
	else if (!strcmp(tokens[0], "unalias"))
	{

		cmd_unalias(tokens);
	}

	else
	{

		cmd_external_commands(tokens);
	}
}

// changes current directory
void cmd_cd(char **tokens)
{

	if (tokens[1] == NULL)
	{
		shell_gethome();
	}
	else if (tokens[2] != NULL)
	{
		printf("Too many arguments\n");
	}

	else if (chdir(tokens[1]) == -1)
		perror(tokens[1]);

	return;
}

// displays internal shell commands instructions
void cmd_help()
{
	printf("\n");
	printf("commands: \n");
	printf("\n");
	printf("cd - changes the current working directory\n");
	printf("\n");
	printf("getpath - print system path\n");
	printf("\n");
	printf("setpath - set system path\n");
	printf("\n");
	printf("history - prints out system history\n");
	printf("\n");
	printf("!! - invoke last command\n");
	printf("\n");
	printf("!<no> - invokes command with number <no> in history\n");
	printf("\n");
	printf("!-<no> - invokes the command current history size - <no>\n");
	printf("\n");
	printf("alias - print all aliases\n");
	printf("\n");
	printf("alias <name> <command> - alias name to be command\n");
	printf("\n");
	printf("unalias <name> - remove any associated alias\n");
	printf("\n");
}

// prints current system path
void cmd_getpath(char **tokens)
{

	if (tokens[1] != NULL)
	{
		printf("Too many arguments\n");
		return;
	}
	const char *path = getenv("PATH");

	if (path)
	{
		printf("%s\n", path);
	}
	else
	{
		perror("Failed");
	}
}

// sets current system path
void cmd_setpath(char **tokens)
{

	if (tokens[1] == NULL)
	{
		printf("Too few arguments\n");
		return;
	}
	if (tokens[2] != NULL)
	{
		printf("Too many arguments\n");
		return;
	}

	if (setenv("PATH", tokens[1], 1) < 0)
	{

		perror("setenv");
	}
}

// prints all contents of history
void cmd_history()
{

	for (int i = 0; i < history_length; i++)
	{
		printf("%d: %s", (i + 1), history[i]);
	}
}

// executes external commands
void cmd_external_commands(char **tokens)
{

	pid_t p = fork();
	// Fork has failed
	if (p < 0)
	{

		perror("fork");
		return;
	}
	// is the parent
	else if (p > 0)
	{

		wait(NULL);
	}
	// is the child
	else if (!p)
	{

		execvp(tokens[0], tokens);
		perror(tokens[0]);
		_exit(127);
	}
}

// adds a command to history
void shell_add_history(char *input)
{

	if (history_length == 20)
	{

		free(history[0]);
		for (int i = 0; i < 19; i++)
		{

			history[i] = history[i + 1];
		}
		history_length--;
	}

	// if there is insufficent memory for the malloc
	if ((history[history_length] = strdup(input)) == NULL)
	{
		perror("strdup");
		printf("error");
		return;
	}
	history_length++;
	return;
}

// checks what history function to use
void cmd_history_commands(char *input, char **tokens)
{

	if (!history_alias)
	{
		shell_tokenise(input, tokens);
		if (tokens[1] != NULL)
		{
			shell_add_history(input);
			printf("Invalid History Invocations. Cannot have parameters.\n");
			return;
		}
	}
	history_alias = 0;
	int num;
	char opr;
	char junk = '0';
	sscanf(input, "%c%d%c", &opr, &num, &junk);
	if (num >= (-20) && num <= 20 && num != 0 && junk == '0')
	{

		if (num > history_length)
		{

			printf("Number provided larger than number of commands currently stored\n");
		}
		else if ((history_length) + num < 0)
		{

			printf("Number provided leads to a negative integer when trying to invoke memory\n");
		}
		else
		{

			if (num > 0)
			{

				// positive!
				cmd_history_positive(num);
			}
			else
			{

				// negative!
				cmd_history_negative(num);
			}

			return;
		}
	}
	if (input[0] == '!' && input[1] == '!' && strlen(input) < 3)
	{
		cmd_history_last();
		return;
	}
	//--------------------------- errors down below -------------------------------------------
	if (num == 32767)
	{
		printf("Only provide an Integer when invoking history\n");
		return;
	}
	if (num > 20)
	{
		printf("Number Provided is larger than possible number of commands stored\n");
		return;
	}

	if (num < -20)
	{
		printf("Number Provided is Smaller than possible number of commands stored\n");
		return;
	}
	if (input[0] == '!' && input[1] == '!')
	{

		printf("When calling !! no other inputs should be provided\n");
		return;
	}
	if (input[0] == '!' && num == 0)
	{

		printf("Must provide an Integer when invoking history\n");
		return;
	}
	if (junk != '0')
	{
		printf("Only provide an Integer when invoking history\n");
		return;
	}
}

// prints the prompt
void shell_printprompt()
{

	char cwd[100];

	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{

		printf("%s> ", cwd);
	}

	else
	{

		perror("getcwd");
		printf("    > ");
	}
}

// gets the users home path
void shell_gethome()
{

	const char *name = "HOME";
	const char *home_path = getenv(name);
	if (chdir(home_path) != 0)
	{
		perror("fail");
	}
}

// restores the users original path
void shell_restore_path(const char *original_path)
{
	// set back things the way they were
	setenv("PATH", original_path, 1);
	const char *path_restored = getenv("PATH");
	printf("%s\n", path_restored);
}

// HISTORY

// prepares and invokes last history command
void cmd_history_last()
{

	if (history_length == 0)
	{
		printf("No commands in history\n");
		return;
	}

	char *original = history[history_length - 1];
	char *input = strdup(original);
	char *tokens[100];
	int length = strlen(input);

	if (input[length - 1] == '\n')
	{
		input[length - 1] = '\0';
	}

	shell_tokenise(input, tokens);

	if (!tokens[0])
	{
		return;
	}

	shell_commands(tokens);
	free(input);
}

// prepares and invokes positive numbered history command
void cmd_history_positive(int num)
{

	char *original = history[num - 1];
	char *input = strdup(original);
	char *tokens[100];
	int length = strlen(input);

	if (input[length - 1] == '\n')
	{
		input[length - 1] = '\0';
	}

	shell_tokenise(input, tokens);
	if (!tokens[0])
	{
		return;
	}

	shell_commands(tokens);
	free(input);
}

// prepares and invokes negative numbered history command
void cmd_history_negative(int num)
{

	int total = history_length;
	int execute = total + num;
	char *original = history[execute];
	char *input = strdup(original);
	char *tokens[100];
	int length = strlen(input);

	if (input[length - 1] == '\n')
	{
		input[length - 1] = '\0';
	}

	shell_tokenise(input, tokens);

	if (!tokens[0])
	{
		return;
	}

	shell_commands(tokens);
	free(input);
}

// saves command on exit
void save_history()
{

	FILE *fptr;
	fptr = fopen(".hist_list.txt", "w");

	for (int i = 0; i < history_length; i++)
	{
		fprintf(fptr, "%s", history[i]);
	}

	fclose(fptr);
}

// loads history if it exists on shell start
void load_history()
{

	FILE *fptr = fopen(".hist_list.txt", "r");

	if (!fptr)
	{
		return;
	}
	else
	{
		char buffer[512];
		while (fgets(buffer, 512, fptr))
		{
			shell_add_history(buffer);
		}
		fclose(fptr);
	}
}

// ALIAS

// adds or overrides an alias
void cmd_alias(char **tokens)
{

	int create = 0;
	for (int i = 0; i < alias_number; i++)
	{

		if (!strcmp(tokens[1], aliases[i]))
		{

			printf("Alias is being overridden.\n");
			create = 1;

			// overriding starts here!!
			int j = 2; // starts at 1 to skip alias command + name
			while (tokens[j] != NULL)
			{
				j++;
			}

			j = j - 2; //- alias command + name

			for (int p = 0; p < j; p++)
			{
				alias_commands[i][p] = malloc(strlen(tokens[p + 2]) + 1);
				strcpy(alias_commands[i][p], tokens[p + 2]);
			}

			alias_commands[i][j] = NULL;
		}
	}

	if (create == 0)
	{
		if (alias_number >= 10)
		{
			printf("Too many aliases set, max is 10.\n");
			return;
		}
		else
		{

			aliases[alias_number] = malloc(strlen(tokens[1]) + 1);
			strcpy(aliases[alias_number], tokens[1]);
			alias_number++;

			int j = 2; // starts at 1 to skip alias command + name
			while (tokens[j] != NULL)
			{
				j++;
			}

			j = j - 2; //- alias command + name

			for (int i = 0; i < j; i++)
			{
				alias_commands[aliascommands_number + offset][i] = malloc(strlen(tokens[i + 2]) + 1); // change to commands number
				strcpy(alias_commands[aliascommands_number + offset][i], tokens[i + 2]);
			}

			alias_commands[aliascommands_number + offset][j] = NULL; // set last bit of alias commands to null
			// set last bit of alias commands to null
			aliascommands_number++;
		}
	}
}

// unaliases a command
void unalias(char *tokens)
{

	if (alias_number == 0)
	{
		printf("Cannot unalias as no aliases set.\n");
		return;
	}

	int found = 0;

	if (found == 0)
	{
		for (int i = 0; aliases[i] != NULL; i++)
		{
			if (!strcmp(tokens, aliases[i]))
			{
				found = 1;

				// i is number of alias to be deleted
				for (int j = i; j < alias_number - 1; j++)
				{
					aliases[j] = aliases[j + 1];
				}

				for (int l = i; l < alias_number; l++)
				{
					for (int m = 0; alias_commands[l][m] != NULL; m++)
					{
						alias_commands[l][m] = alias_commands[l + 1][m];
					}
				}

				for (int p = 0; alias_commands[alias_number][p] != NULL; p++)
				{
					alias_commands[alias_number][p] = NULL;
					free(alias_commands[alias_number][p]);
				}

				alias_number--;
				aliascommands_number--;
			}
		}
	}
}

// calls unalise()
void cmd_unalias(char **tokens)
{
	if (tokens[1] == NULL)
	{
		printf("Must provide a name to unalias\n");
		return;
	}
	unalias(tokens[1]);
}

// saves aliases on shell close
void save_aliases()
{
	FILE *fptr;

	fptr = fopen(".aliases.txt", "w");

	for (int i = 0; i < alias_number; i++)
	{
		fprintf(fptr, "%s ", aliases[i]);

		for (int j = 0; alias_commands[i][j] != NULL; j++)
		{
			if (alias_commands[i][j] != NULL)
			{
				fprintf(fptr, "%s ", alias_commands[i][j]);
			}
		}
		fprintf(fptr, "\n");
	}
	fclose(fptr);
}

// load aliases on shell start
void load_aliases()
{

	FILE *fptr = fopen(".aliases.txt", "r");

	if (!fptr)
	{
		return;
	}
	else
	{
		char *tokens[512];
		char buffer[512];
		int i = 0;
		while (fgets(buffer, 512, fptr))
		{

			shell_tokenise(buffer, tokens);

			if (tokens[0] == NULL || tokens[1] == NULL)
			{

				printf("Invalid token structure\n");
				for (int j = 0; j < (alias_number + offset); j++)
				{

					unalias(aliases[j]);
				}
				break;
				break;
			}

			aliases[i] = malloc(strlen(tokens[0]) + 1);
			strcpy(aliases[i], tokens[0]);

			// tokens should have at least two elements, clear file if any error

			for (int j = 1; tokens[j] != NULL; j++)
			{
				alias_commands[i][j - 1] = malloc(strlen(tokens[j]) + 1);
				strcpy(alias_commands[i][j - 1], tokens[j]);
			}
			alias_number++;

			i++;
			offset++;
		}
		fclose(fptr);
	}
}
